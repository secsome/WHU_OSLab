#include <fs/fd.h>
#include <fs/core.h>
#include <fs/device.h>

#include <sys/sendrecv.h>
#include <sys/proc.h>

#include <lib/assert.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <lib/algorithm>

extern u8* fs_buffer;

file_descriptor_t fs_file_descriptor_table[FS_NUM_FD];

static inode_t* fd_create_file(const char* path, int flags);
static int fd_alloc_imap_bit(int device);
static int fd_alloc_smap_bit(int device, int sector_count);
static inode_t* fd_new_inode(int device, int inode_index, int start_sector);
static void fd_new_dir_entry(inode_t* dir_node, int inode_index, const char* filename);

int open(const char* path, int flags)
{
    message_t msg;
    msg.type = SR_MSGTYPE_OPEN;
    msg.m_open.path = path;
    msg.m_open.flags = flags;
    msg.m_open.path_length = strlen(path);

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_open(const message_t& msg)
{
    int fd = -1;
    char filepath[MAX_PATH];

    const int flags = msg.m_open.flags;
    const u32 length = msg.m_open.path_length;

    assert(length + 1 < sizeof(filepath));

    memcpy(va2la(TASK_FILESYSTEM, filepath), va2la(msg.source, msg.m_open.path), length);
    filepath[length] = '\0';

    // find a free slot in process fd_table
    auto& process = proc_table[msg.source];
    for (int i = 0; i < NUM_PROCS; ++i)
    {
        if (process.fd_table[i] == nullptr)
        {
            fd = i;
            break;
        }
    }
    if (fd < 0 || fd >= FS_NUM_FILES)
        panic("FS::open: no available fd (PID: %d)", msg.source);
    
    // find a free slot in f_desc_table
    int fd_index = -1;
    for (int i = 0; i < FS_NUM_FD; ++i)
    {
        if (fs_file_descriptor_table[i].inode == 0)
        {
            fd_index = i;
            break;
        }
    }
    assert(fd_index >= 0 && fd_index < FS_NUM_FD);

    const int inode_index = fs_search_file(filepath);

    inode_t* in = nullptr;
    if (flags & FS_FDFLAGS_CREAT)
    {
        if (inode_index)
        {
            printl("file exists.\n");
            return -1;
        }
        else
            in = fd_create_file(filepath, flags);
    }
    else
    {
        assert(flags & FS_FDFLAGS_RDWR);

        char filename[MAX_PATH];
        inode_t* dir_node;
        if (fs_strip_path(filename, filepath, &dir_node) != 0)
            return -1;
        in = fs_get_inode(dir_node->device, inode_index);
    }

    if (in)
    {
        auto& item = fs_file_descriptor_table[fd_index];
        process.fd_table[fd] = &item;
        item.inode = in;
        item.mode = flags;
        item.position = 0;

        int imode = in->mode & FS_INODE_TYPE_MASK;
        if (imode == FS_INODE_CHAR_SPECIAL)
        {
            message_t driver_msg;
            driver_msg.type = SR_MSGTYPE_DEVOPEN;
            driver_msg.m_device.device = MINOR_DEV(in->start_sector);
            assert(MAJOR_DEV(in->start_sector) == 4);
            assert(device_driver_map[MAJOR_DEV(in->start_sector)] != FS_INVALID_DRIVER);
            sendrecv(SR_MODE_BOTH, device_driver_map[MAJOR_DEV(in->start_sector)], &driver_msg);
        }
        else if (imode == FS_INODE_DIRECTORY)
            assert(in->index == FS_INODE_ROOT);
        else
            assert(in->mode == FS_INODE_REGULAR);
    }
    else
        return -1;
    
    return fd;
}

int close(int fd)
{
    message_t msg;
    msg.type = SR_MSGTYPE_CLOSE;
    msg.m_int32 = fd;

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_close(const message_t& msg)
{
    if (msg.m_int32 < 0 || msg.m_int32 >= FS_NUM_FD)
        return -1;
    auto& fd = proc_table[msg.source].fd_table[msg.m_int32];
    fs_put_inode(fd->inode);
    fd->inode = 0;
    fd = nullptr;
    return 0;
}

int read(int fd, void* buf, size_t count)
{
    message_t msg;
    msg.type = SR_MSGTYPE_READ;
    msg.m_readwrite.fd = fd;
    msg.m_readwrite.buffer = buf;
    msg.m_readwrite.count = count;

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int write(int fd, const void* buf, size_t count)
{
    message_t msg;
    msg.type = SR_MSGTYPE_WRITE;
    msg.m_readwrite.fd = fd;
    msg.m_readwrite.buffer = const_cast<void*>(buf);
    msg.m_readwrite.count = count;

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_readwrite(const message_t& msg)
{
    const int fd = msg.m_readwrite.fd;
    void* buf =  msg.m_readwrite.buffer;
    size_t count = msg.m_readwrite.count;
    
    auto& caller = proc_table[msg.source];
    auto& pfd = caller.fd_table[fd];
    assert(pfd >= &fs_file_descriptor_table[0] && pfd < &fs_file_descriptor_table[FS_NUM_FD]);

    if (nullptr == pfd)
        return 0;
    if ((pfd->mode & FS_FDFLAGS_RDWR) == 0)
        return 0;
    
    const int pos = pfd->position;
    const auto node = pfd->inode;
    const int imode = pfd->inode->mode & FS_INODE_TYPE_MASK;
    assert(node >= &fs_inode_table[0] && node < &fs_inode_table[FS_NUM_INODE]);

	if (imode == FS_INODE_CHAR_SPECIAL)
    {
        message_t driver_msg;
		driver_msg.type = msg.type == SR_MSGTYPE_READ ? SR_MSGTYPE_DEVREAD : SR_MSGTYPE_DEVWRITE;
        const int device =node->start_sector;
		assert(MAJOR_DEV(device) == 4);
        driver_msg.m_device.device = MINOR_DEV(device);
        driver_msg.m_device.buffer = buf;
        driver_msg.m_device.count = count;
        driver_msg.m_device.process_index = msg.source;
        assert(device_driver_map[MAJOR_DEV(device)] != FS_INVALID_DRIVER);
        sendrecv(SR_MODE_BOTH, device_driver_map[MAJOR_DEV(device)], &driver_msg);
        assert(driver_msg.m_int32 == static_cast<int>(count));
        return driver_msg.m_int32;
	}
	else
    {
        assert(imode == FS_INODE_REGULAR || imode == FS_INODE_DIRECTORY);
        assert(msg.type == SR_MSGTYPE_READ || msg.type == SR_MSGTYPE_WRITE);

        size_t pos_end;
        if (msg.type == SR_MSGTYPE_READ)
            pos_end = std::min(pos + count, node->size);
        else
            pos_end = std::min(pos + count, node->num_sectors * HD_SECTOR_SIZE);
        
        int offset = pos % HD_SECTOR_SIZE;
        const int sector_min = node->start_sector + (pos >> HD_SECTOR_SIZE_SHIFT);
        const int sector_max = node->start_sector + (pos_end >> HD_SECTOR_SIZE_SHIFT);

        const int chunk = std::min(sector_max - sector_min + 1, FS_BUFFER_SIZE >> HD_SECTOR_SIZE_SHIFT);

        int bytes_done = 0;
        int bytes_left = count;
		
		for (int i = sector_min; i <= sector_max; i += chunk)
        {
			// read/write this amount of bytes every time
			const auto unit_size = std::min(bytes_left, chunk * HD_SECTOR_SIZE - offset);
			fs_readwrite_sector(SR_MSGTYPE_DEVREAD, node->device, i * HD_SECTOR_SIZE, chunk * HD_SECTOR_SIZE, TASK_FILESYSTEM, fs_buffer);
            if (msg.type == SR_MSGTYPE_READ)
                memcpy(va2la(msg.source, reinterpret_cast<char*>(buf) + bytes_done), fs_buffer + offset, unit_size);
            else
            {
                memcpy(fs_buffer + offset, va2la(msg.source, reinterpret_cast<char*>(buf) + bytes_done), unit_size);
                fs_readwrite_sector(SR_MSGTYPE_DEVWRITE, node->device, i * HD_SECTOR_SIZE, chunk * HD_SECTOR_SIZE, TASK_FILESYSTEM, fs_buffer);
            }

			offset = 0;
			bytes_done += unit_size;
			pfd->position += unit_size;
			bytes_left -= unit_size;
		}

		if (static_cast<u32>(pfd->position) > node->size)
        {
            node->size = pfd->position;
            // write the updated i-node back to disk
			fs_write_inode(node);
		}

		return bytes_done;
	}
}

int unlink(const char* pathname)
{
    message_t msg;
    msg.type = SR_MSGTYPE_UNLINK;
    msg.m_string.str = pathname;
    msg.m_string.length = strlen(pathname);

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_unlink(const message_t& msg)
{
    char pathname[MAX_PATH];

	// get parameters from the message
	const auto length = msg.m_string.length; // length of filename
	assert(length < MAX_PATH);

    memcpy(pathname, va2la(msg.source, msg.m_string.str), length);
	pathname[length] = 0;

	if (!strcmp(pathname , "/"))
    {
		printl("FS:do_unlink():: cannot unlink the root\n");
		return -1;
	}

	const int inode_index = fs_search_file(pathname);
	if (inode_index == FS_INODE_INVALID)
    {
		printl("FS::do_unlink():: search_file() returns invalid inode: %s\n", pathname);
		return -1;
	}

	char filename[MAX_PATH];
	inode_t* dir_node;
	if (fs_strip_path(filename, pathname, &dir_node) != 0)
		return -1;

	inode_t* node = fs_get_inode(dir_node->device, inode_index);

	if (node->mode != FS_INODE_REGULAR) // can only remove regular files
    {
		printl("cannot remove file %s, because it is not a regular file.\n", pathname);
		return -1;
	}

	if (node->ref_count > 1) // the file was opened
    {
		printl("cannot remove file %s, because node->ref_cnt is %d.\n", pathname, node->ref_count);
		return -1;
	}

    const auto sb = fs_get_super_block(node->device);
    // free the bit in imap
    {
        const int byte_idx = inode_index / 8;
        const int bit_idx = inode_index % 8;
	    assert(byte_idx < HD_SECTOR_SIZE);	// we have only one i-map sector

	    // read sector 2 (skip bootsect and superblk):
	    fs_read_sector(node->device, 2, fs_buffer);
	    assert(fs_buffer[byte_idx % HD_SECTOR_SIZE] & (1 << bit_idx));
	    fs_buffer[byte_idx % HD_SECTOR_SIZE] &= ~(1 << bit_idx);
	    fs_write_sector(node->device, 2, fs_buffer);
    }

	// free the bits in s-map
    {
        const int bit_idx = node->start_sector - sb->num_first_sector + 1;
        const int byte_idx = bit_idx / 8;
        int bits_left = node->num_sectors;
        const int byte_cnt = (bits_left - (8 - (bit_idx % 8))) / 8;

        // current sector index
        int current_sector = 2 + sb->num_imap_sectors + byte_idx / HD_SECTOR_SIZE;
        fs_read_sector(node->device, current_sector, fs_buffer);
        // clear the first byte
        for (int i = bit_idx % 8; (i < 8) && bits_left; ++i, --bits_left)
        {
            assert((fs_buffer[byte_idx % HD_SECTOR_SIZE] >> i & 1) == 1);
            fs_buffer[byte_idx % HD_SECTOR_SIZE] &= ~(1 << i);
        }

        // clear bytes from the second byte to the second to last
        int i = (byte_idx % HD_SECTOR_SIZE) + 1;
        for (int k = 0; k < byte_cnt; ++k, ++i, bits_left -= 8)
        {
            if (i == HD_SECTOR_SIZE)
            {
                i = 0;
                fs_write_sector(node->device, current_sector, fs_buffer);
                fs_read_sector(node->device, ++current_sector, fs_buffer);
            }
            assert(fs_buffer[i] == 0xFF);
            fs_buffer[i] = 0;
        }

        // clear the last byte
        if (i == HD_SECTOR_SIZE)
        {
            i = 0;
            fs_write_sector(node->device, current_sector, fs_buffer);
            fs_read_sector(node->device, ++current_sector, fs_buffer);
        }

        unsigned char mask = ~((unsigned char)(~0) << bits_left);
        assert((fs_buffer[i] & mask) == mask);
        fs_buffer[i] &= (~0) << bits_left;
        fs_write_sector(node->device, current_sector, fs_buffer);
    }

    // clear the inode itself
    {
        node->mode = 0;
        node->size = 0;
        node->start_sector = 0;
        node->num_sectors = 0;
        fs_write_inode(node);
        // release slot in inode_table
        fs_put_inode(node);
    }

    // set the inode_index to 0 in the directory entry
    {
        const int dir_block0_index = dir_node->start_sector;
        const int dir_blocks_count = (dir_node->size + HD_SECTOR_SIZE - 1) / HD_SECTOR_SIZE;
        const int dir_entries_count = dir_node->size / FS_DIRENTRY_SIZE;

        int m = 0;
        dir_entry_t* pde = nullptr;
        bool flag = false;
        int dir_size = 0;

        for (int i = 0; i < dir_blocks_count; ++i)
        {
            fs_read_sector(dir_node->device, dir_block0_index + i, fs_buffer);
            pde = reinterpret_cast<dir_entry_t*>(fs_buffer);
            for (int j = 0; j < FS_DIRENTRY_PER_SECTOR; ++j, ++pde)
            {
                if (++m > dir_entries_count)
                    break;
                
                if (pde->inode_index == inode_index)
                {
                    memset(pde, 0, FS_DIRENTRY_SIZE);
                    fs_write_sector(dir_node->device, dir_block0_index + i, fs_buffer);
                    flag = true;
                    break;
                }

                if (pde->inode_index != FS_INODE_INVALID)
                    dir_size += FS_DIRENTRY_SIZE;
            }

            if (m > dir_entries_count || flag) // all entries have been iterated OR file is found
                break;
        }

        assert(flag);

        if (m == dir_entries_count) // the file is the last one in the dir
        {
            dir_node->size = dir_size;
            fs_write_inode(dir_node);
        }
    }

	return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
    message_t msg;
    msg.type = SR_MSGTYPE_LSEEK;
    msg.m_lseek.fd = fd;
    msg.m_lseek.offset = offset;
    msg.m_lseek.whence = whence;

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_lseek(const message_t& msg)
{
    panic("%s is not implemented yet.", __FUNCTION__);
    return -1;
}

int stat(const char* pathname)
{
    message_t msg;
    msg.type = SR_MSGTYPE_STAT;
    msg.m_string.str = pathname;
    msg.m_string.length = strlen(pathname);

    sendrecv(SR_MODE_BOTH, TASK_FILESYSTEM, &msg);
    assert(msg.type == SR_MSGTYPE_SYSCALL_RET);
    return msg.m_int32;
}

int fs_do_stat(const message_t& msg)
{
    panic("%s is not implemented yet.", __FUNCTION__);
    return -1;
}

int creat(const char* pathname)
{
    return open(pathname, O_CREAT);
}

int remove(const char* pathname)
{
    // Not implemented for directories
    return unlink(pathname);
}

static inode_t* fd_create_file(const char* path, int flags)
{
    char filename[MAX_PATH];
    inode_t* dir_node;

    if (fs_strip_path(filename, path, &dir_node) != 0)
        return nullptr;
    
    int inode_index = fd_alloc_imap_bit(dir_node->device);
    int free_sector_index = fd_alloc_smap_bit(dir_node->device, FS_DEFAULT_FILE_SECTORS);
    inode_t* new_node = fd_new_inode(dir_node->device, inode_index, free_sector_index);

    fd_new_dir_entry(dir_node, new_node->index, filename);

    return new_node;
}

static int fd_alloc_imap_bit(int dev)
{
	int inode_index = 0;
	int imap_block0_index = 1 + 1; /* 1 boot sector & 1 super block */
	auto sb = fs_get_super_block(dev);

	for (size_t i = 0; i < sb->num_imap_sectors; ++i)
    {
		fs_read_sector(dev, imap_block0_index + i, fs_buffer);
		for (size_t j = 0; j < HD_SECTOR_SIZE; ++j)
        {
			// skip `11111111' bytes
			if (fs_buffer[j] == 0xFF)
				continue;
			// skip `1' bits
            int k = 0;
            while ((fs_buffer[j] >> k) & 1)
                ++k;
			// i: sector index; j: byte index; k: bit index
			inode_index = (i * HD_SECTOR_SIZE + j) * 8 + k;
			fs_buffer[j] |= (1 << k);
			
            // write the bit to imap
			fs_write_sector(dev, imap_block0_index + i, fs_buffer);
			break;
		}

		return inode_index;
	}

	// no free bit in imap
	panic("inode-map is probably full.\n");

	return 0;
}

static int fd_alloc_smap_bit(int device, int sector_count)
{
	super_block_t* sb = fs_get_super_block(device);

	int smap_block0_index = 2 + sb->num_imap_sectors;
	int free_sector_index = 0;

	for (size_t i = 0; i < sb->num_smap_sectors; ++i)
    {
		fs_read_sector(device, smap_block0_index + i, fs_buffer);

		// byte offset in current sect
		for (int j = 0; j < HD_SECTOR_SIZE && sector_count > 0; ++j)
        {
			int k = 0;
			if (!free_sector_index)
            {
				// loop until a free bit is found
				if (fs_buffer[j] == 0xFF) 
                    continue;
                
                while ((fs_buffer[j] >> k) & 1)
                    ++k;
				
                free_sector_index = (i * HD_SECTOR_SIZE + j) * 8 + k - 1 + sb->num_first_sector;
			}

			for (; k < 8; ++k) // repeat till enough bits are set
            {
				assert(((fs_buffer[j] >> k) & 1) == 0);
				fs_buffer[j] |= (1 << k);
				if (--sector_count == 0)
					break;
			}
		}

		if (free_sector_index) // free bit found, write the bits to smap
			fs_write_sector(device, smap_block0_index + i, fs_buffer);

		if (sector_count == 0)
			break;
	}

	assert(sector_count == 0);

	return free_sector_index;
}

static inode_t* fd_new_inode(int device, int inode_index, int start_sector)
{
    inode_t* new_node = fs_get_inode(device, inode_index);
    new_node->mode = FS_INODE_REGULAR;
    new_node->size = 0;
    new_node->start_sector = start_sector;
    new_node->num_sectors = FS_DEFAULT_FILE_SECTORS;

    new_node->device = device;
    new_node->ref_count = 1;
    new_node->index = inode_index;

    // write to the inode array
    fs_write_inode(new_node);

    return new_node;
}

static void fd_new_dir_entry(inode_t* dir_node, int inode_index, const char* filename)
{
    // write the dir_entry
    int dir_block0_index = dir_node->start_sector;
    int dir_blocks_count = (dir_node->size + HD_SECTOR_SIZE) / HD_SECTOR_SIZE;

    int dir_entries_count = dir_node->size / FS_DIRENTRY_SIZE;
	int m = 0;
	dir_entry_t* pde;
	dir_entry_t* new_de = nullptr;

    int i;
	for (i = 0; i < dir_blocks_count; ++i)
    {
        fs_read_sector(dir_node->device, dir_block0_index + i, fs_buffer);

		pde = (dir_entry_t *)fs_buffer;
		for (int j = 0; j < FS_DIRENTRY_PER_SECTOR; ++j, ++pde)
        {
			if (++m > dir_entries_count)
				break;

			if (pde->inode_index == 0) // it's a free slot
            {
				new_de = pde;
				break;
			}
		}
		if (m > dir_entries_count || new_de)
			break;
	}
	if (!new_de) // reached the end of the dir
    {
		new_de = pde;
		dir_node->size += FS_DIRENTRY_SIZE;
	}
	new_de->inode_index = inode_index;
	strcpy(new_de->filename, filename);

	// write dir block -- ROOT dir block
	fs_write_sector(dir_node->device, dir_block0_index + i, fs_buffer);

	// update dir inode
	fs_write_inode(dir_node);
}