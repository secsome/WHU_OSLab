#include <fs/core.h>
#include <fs/device.h>
#include <fs/fd.h>

#include <sys/sendrecv.h>

#include <lib/assert.h>
#include <lib/string.h>

super_block_t fs_superblocks[FS_NUM_SUPERBLOCK];
inode_t fs_inode_table[FS_NUM_INODE];
inode_t* fs_inode_root;

extern u8* fs_buffer;

void fs_read_sector(int device, int sector, void* buffer)
{
    fs_readwrite_sector(SR_MSGTYPE_DEVREAD, device, sector * HD_SECTOR_SIZE, HD_SECTOR_SIZE, TASK_FILESYSTEM, buffer);
}

void fs_write_sector(int device, int sector, const void* buffer)
{
    fs_readwrite_sector(SR_MSGTYPE_DEVWRITE, device, sector * HD_SECTOR_SIZE, HD_SECTOR_SIZE, TASK_FILESYSTEM, const_cast<void*>(buffer));
}

void fs_readwrite_sector(int type, int device, u64 position, int count, int proccess_index, void* buffer)
{
    message_t msg;
    msg.type = type;
    msg.m_device.device = MINOR_DEV(device);
    msg.m_device.position = position;
    msg.m_device.buffer = buffer;
    msg.m_device.count = count;
    msg.m_device.process_index = proccess_index;

    const auto driver_index = device_driver_map[MAJOR_DEV(device)];
    assert(driver_index != FS_INVALID_DRIVER);

    sendrecv(SR_MODE_BOTH, driver_index, &msg);
}

super_block_t* fs_get_super_block(int device)
{
    for (auto& sb : fs_superblocks)
    {
        if (sb.superblock_device == device)
            return &sb;
    }

	panic("super block of devie %d not found.\n", device);

	return 0;
}

inode_t* fs_get_inode(int device, int inode)
{
    if (inode == FS_INODE_INVALID)
        return nullptr;
    
    inode_t* q = nullptr;

    for (auto& p : fs_inode_table)
    {
        if (p.ref_count) // not a free slot
        {
            if (p.device == device && p.index == inode)
            {
                ++p.ref_count;
                return &p;
            }  
        }
        else
        {
            if (nullptr == q)
                q = &p;
        }
    }

    if (nullptr == q)
        panic("inode table is full!");
    
    q->device = device;
    q->index = inode;
    q->ref_count = 1;

    const auto sb = fs_get_super_block(device);
    const int block_index = 2 + sb->num_imap_sectors + sb->num_smap_sectors + ((inode - 1) / FS_INODE_PER_SECTOR);
    fs_read_sector(device, block_index, fs_buffer);
    auto node = reinterpret_cast<inode_t*>(fs_buffer + ((inode - 1) % FS_INODE_PER_SECTOR) * FS_INODE_SIZE);
	q->mode = node->mode;
	q->size = node->size;
	q->start_sector = node->start_sector;
	q->num_sectors = node->num_sectors;
	
    return q;
}

void fs_write_inode(inode_t* node)
{
    const auto sb = fs_get_super_block(node->device);
    const int block_index = 2 + sb->num_imap_sectors + sb->num_smap_sectors + ((node->index - 1) / FS_INODE_PER_SECTOR);
    fs_read_sector(node->device, block_index, fs_buffer);
    auto pnode = reinterpret_cast<inode_t*>(fs_buffer + (((node->index - 1) % FS_INODE_PER_SECTOR) * FS_INODE_SIZE));
    pnode->mode = node->mode;
    pnode->size = node->size;
    pnode->start_sector = node->start_sector;
    pnode->num_sectors = node->num_sectors;
    fs_write_sector(node->device, block_index, fs_buffer);
}

void fs_put_inode(inode_t* node)
{
    assert(node->ref_count > 0);
    --node->ref_count;
}

int fs_search_file(const char* path)
{
    char filename[MAX_PATH] = { 0 };
    inode_t* dir_node;
    if (fs_strip_path(filename, path, &dir_node) != 0)
        return 0;
    
    if (filename[0] == 0) // path: "/"
        return dir_node->num_sectors;

    // search the dir for the file
    const int dir_block0_index = dir_node->start_sector;
    const int dir_blocks_count = (dir_node->size + HD_SECTOR_SIZE - 1) / HD_SECTOR_SIZE;
    const int dir_entries_count = dir_node->size / FS_DIRENTRY_SIZE;

	int m = 0;
    for (int i = 0; i < dir_blocks_count; ++i)
    {
        fs_read_sector(dir_node->device, dir_block0_index + i, fs_buffer);
        auto pde = reinterpret_cast<dir_entry_t*>(fs_buffer);
        for (int j = 0; j < FS_DIRENTRY_PER_SECTOR; ++j, ++pde)
        {
            if (!memcmp(filename, pde->filename, FS_MAX_FILENAME))
                return pde->inode_index;
            if (++m > dir_entries_count)
                break;
        }
    }

	// file not found
	return 0;
}

int fs_strip_path(char* filename, const char* filepath, inode_t** node)
{
    const char* s = filepath;
    char* t = filename;

    if (s == 0)
		return -1;

	if (*s == '/')
		s++;

	while (*s) // check each character
    {
		if (*s == '/')
			return -1;
		*t++ = *s++;
		// if filename is too long, just truncate it
		if (t - filename >= FS_MAX_FILENAME)
			break;
	}
	*t = 0;

	*node = fs_inode_root;

    return 0;
}