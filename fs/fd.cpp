#include <fs/fd.h>
#include <fs/core.h>
#include <fs/device.h>

#include <sys/sendrecv.h>
#include <sys/proc.h>

#include <lib/assert.h>
#include <lib/string.h>
#include <lib/printf.h>

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