#include <fs/core.h>
#include <fs/device.h>

#include <sys/sendrecv.h>
#include <sys/console.h>
#include <sys/debug.h>

#include <lib/printf.h>
#include <lib/assert.h>
#include <lib/string.h>

static void init_fs();
static void mkfs();
static void read_superblock(int device);

u8* fs_buffer = reinterpret_cast<u8*>(0x600000u);

extern int fs_do_open(const message_t& msg);
extern int fs_do_close(const message_t& msg);
extern int fs_do_readwrite(const message_t& msg);

void task_fs()
{
    printl("Task FS begins.\n");
    init_fs();
    
    while (true)
    {
        message_t msg;
        sendrecv(SR_MODE_RECEIVE, SR_TARGET_ANY, &msg);
        const u32 target = msg.source;

        switch (msg.type)
        {
        case SR_MSGTYPE_OPEN:
            msg.m_int32 = fs_do_open(msg);
            break;
        
        case SR_MSGTYPE_CLOSE:
            msg.m_int32 = fs_do_close(msg);
            break;
        
        case SR_MSGTYPE_READ:
        case SR_MSGTYPE_WRITE:
            msg.m_int32 = fs_do_readwrite(msg);
            break;

        default:
            debug_dump_message("FS::unknown message:", &msg);
            assert(0);
            break;
        }

        msg.type = SR_MSGTYPE_SYSCALL_RET;
        sendrecv(SR_MODE_SEND, target, &msg);
    }
};

static void init_fs()
{
    for (auto& fd : fs_file_descriptor_table)
        memset(&fd, 0, sizeof(fd));
    
    for (auto& inode : fs_inode_table)
        memset(&inode, 0, sizeof(inode));
    
    for (auto& sb : fs_superblocks)
        sb.superblock_device = FS_DEV_NONE;

    // open the device: hard disk
    message_t msg;
    msg.type = SR_MSGTYPE_DEVOPEN;
    msg.m_device.device = MINOR_DEV(FS_ROOT_DEVICE);
    const auto device = device_driver_map[MAJOR_DEV(FS_ROOT_DEVICE)];
    assert(device != FS_INVALID_DRIVER);
    sendrecv(SR_MODE_BOTH, device, &msg);

	mkfs();

    // load super block of ROOT
    read_superblock(FS_ROOT_DEVICE);

    const auto sb = fs_get_super_block(FS_ROOT_DEVICE);
    assert(sb->magic == FS_MAGIC_V1);

    fs_inode_root = fs_get_inode(FS_ROOT_DEVICE, FS_INODE_ROOT);
}

static void read_superblock(int device)
{
    message_t msg;
    msg.type = SR_MSGTYPE_DEVREAD;
    msg.m_device.device = MINOR_DEV(device);
    msg.m_device.position = 1 * HD_SECTOR_SIZE;
    msg.m_device.buffer = fs_buffer;
    msg.m_device.count = HD_SECTOR_SIZE;
    msg.m_device.process_index = TASK_FILESYSTEM;
    assert(device_driver_map[MAJOR_DEV(device)] != FS_INVALID_DRIVER);
    sendrecv(SR_MODE_BOTH, device_driver_map[MAJOR_DEV(device)], &msg);

    // find a free slot in super_block
    int i;
    for (i = 0; i < FS_NUM_SUPERBLOCK; ++i)
    {
        if (fs_superblocks[i].superblock_device == FS_DEV_NONE)
            break;
    }
	if (i == FS_NUM_SUPERBLOCK)
		panic("super_block slots used up");

	assert(i == 0); // currently we use only the 1st slot

	auto sb = reinterpret_cast<super_block_t*>(fs_buffer);
	fs_superblocks[i] = *sb;
	fs_superblocks[i].superblock_device = device;
}

static void mkfs()
{
    message_t msg;

    constexpr auto bits_per_sector = HD_SECTOR_SIZE * 8;

    partition_info_t geo;
    msg.type = SR_MSGTYPE_DEVIOCTL;
    msg.m_device.device = MINOR_DEV(FS_ROOT_DEVICE);
    msg.m_device.request = HD_IOCTL_GET_GEO;
    msg.m_device.buffer = &geo;
    msg.m_device.process_index = TASK_FILESYSTEM;

    const auto device = device_driver_map[MAJOR_DEV(FS_ROOT_DEVICE)];
    assert(device != FS_INVALID_DRIVER);
    sendrecv(SR_MODE_BOTH, device, &msg);

    printl("dev size: 0x%x sectors\n", geo.size);

    // Super block
    super_block_t sb;
	sb.magic = FS_MAGIC_V1;
	sb.num_inodes = bits_per_sector;
	sb.num_inode_sectors = sb.num_inodes * FS_INODE_SIZE / HD_SECTOR_SIZE;
	sb.num_sectors = geo.size; // partition size in sector
	sb.num_imap_sectors = 1;
	sb.num_smap_sectors = sb.num_sectors / bits_per_sector + 1;
	sb.num_first_sector = 1 + 1 + // boot sector & super block
		sb.num_imap_sectors + sb.num_smap_sectors + sb.num_inode_sectors;
	sb.root_inode = FS_INODE_ROOT;
	sb.inode_size = FS_INODE_SIZE;
	
    sb.inode_isize_offset = offsetof(inode_t, size);
    sb.inode_start_offset = offsetof(inode_t, start_sector);
    sb.dir_entry_size = FS_DIRENTRY_SIZE;
    sb.dir_entry_inode_offset = offsetof(dir_entry_t, inode_index);
    sb.dir_entry_fname_offset = offsetof(dir_entry_t, filename);
    
	memset(fs_buffer, 0x90, HD_SECTOR_SIZE);
	memcpy(fs_buffer, &sb, FS_SUPER_BLOCK_SIZE);

	/* write the super block */
    fs_write_sector(FS_ROOT_DEVICE, 1, fs_buffer);

	printl("devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
	       "        inodes:0x%x00, 1st_sector:0x%x00\n", 
	       geo.base * 2,
	       (geo.base + 1) * 2,
	       (geo.base + 1 + 1) * 2,
	       (geo.base + 1 + 1 + sb.num_imap_sectors) * 2,
	       (geo.base + 1 + 1 + sb.num_imap_sectors + sb.num_smap_sectors) * 2,
	       (geo.base + sb.num_first_sector) * 2);
    
    // Inode map
    memset(fs_buffer, 0, HD_SECTOR_SIZE);
    for (int i = 0; i < (NUM_CONSOLES + 2); ++i)
		fs_buffer[0] |= 1 << i;
    
    assert(fs_buffer[0] == 0x1F);
    fs_write_sector(FS_ROOT_DEVICE, 2, fs_buffer);

    // Sector map
    memset(fs_buffer, 0, HD_SECTOR_SIZE);
	int sector_index = FS_DEFAULT_FILE_SECTORS + 1;

    memset(fs_buffer, 0xFF, sector_index / 8);
    for (int i = 0; i < sector_index % 8; ++i)
        fs_buffer[sector_index / 8] |= 1 << i;

    fs_write_sector(FS_ROOT_DEVICE, 2 + sb.num_imap_sectors, fs_buffer);

	// zeromemory the rest sector-map
	memset(fs_buffer, 0, HD_SECTOR_SIZE);
	for (size_t i = 1; i < sb.num_smap_sectors; ++i)
        fs_write_sector(FS_ROOT_DEVICE, 2 + sb.num_imap_sectors + i, fs_buffer);
    
    // inodes
    // root inode
	memset(fs_buffer, 0, HD_SECTOR_SIZE);
    inode_t* inode = reinterpret_cast<inode_t*>(fs_buffer);
    inode->mode = FS_INODE_DIRECTORY;
    inode->size = FS_DIRENTRY_SIZE * 4; // 4 files: `.', `dev_tty0', `dev_tty1', `dev_tty2'
    inode->start_sector = sb.num_first_sector;
    inode->num_sectors = FS_DEFAULT_FILE_SECTORS;

	// inode of `/dev_tty0~2'
	for (int i = 0; i < NUM_CONSOLES; ++i)
    {
        inode = reinterpret_cast<inode_t*>(fs_buffer + (i + 1) * FS_INODE_SIZE);
		inode->mode = FS_INODE_CHAR_SPECIAL;
        inode->size = 0;
        inode->start_sector = MAKE_DEV(FS_DEV_CHAR_TTY, i);
        inode->num_sectors = 0;
	}
    fs_write_sector(FS_ROOT_DEVICE, 2 + sb.num_imap_sectors + sb.num_smap_sectors, fs_buffer);

    // '/'
	memset(fs_buffer, 0, HD_SECTOR_SIZE);
    dir_entry_t* dir = reinterpret_cast<dir_entry_t*>(fs_buffer);
    dir->inode_index = 1;
    strcpy(dir->filename, ".");

	// dir entries of `/dev_tty0~2'
	for (int i = 0; i < NUM_CONSOLES; ++i) {
		++dir;
		dir->inode_index = i + 2; // dev_tty0's inode_index is 2
		sprintf(dir->filename, "dev_tty%d", i);
	}
    fs_write_sector(FS_ROOT_DEVICE, sb.num_first_sector, fs_buffer);
}