#pragma once

#include <sys/const.h>
#include <sys/type.h>
#include <sys/harddisk.h>

#include <fs/config.h>

HEADER_CPP_BEGIN

typedef int dev_driver_map_t;

typedef struct super_block_t
{
	u32	magic;
    u32 num_inodes;
    u32 num_sectors;
    u32 num_imap_sectors;
    u32 num_smap_sectors;
    u32 num_first_sector;
    u32 num_inode_sectors;
    u32 root_inode;
    u32 inode_size;
    u32 inode_isize_offset;
    u32 inode_start_offset;
    u32 dir_entry_size;
    u32 dir_entry_inode_offset;
    u32 dir_entry_fname_offset;

	// The following item(s) are only present in memory
	int	superblock_device; // the super block's home device
} super_block_t;
extern super_block_t fs_superblocks[FS_NUM_SUPERBLOCK];

typedef struct inode_t
{
    u32 mode;
    u32 size;
    u32 start_sector;
    u32 num_sectors;
    u8 unused[16];

    // The following item(s) are only present in memory
    int device; 
    int ref_count; // how many procs share this inode
    int index;
} inode_t;
extern inode_t fs_inode_table[FS_NUM_INODE];
extern inode_t* fs_inode_root;

// imodes : octal, lower 12 bits reserved
enum
{
    FS_INODE_TYPE_MASK = 0170000,
    FS_INODE_REGULAR = 0100000,
    FS_INODE_BLOCK_SPECIAL = 0060000,
    FS_INODE_DIRECTORY = 0040000,
    FS_INODE_CHAR_SPECIAL = 0020000,
    FS_INODE_NAMED_PIPE = 0010000
};
#define FS_INODE_MODE(mode) ((mode) & FS_INODE_TYPE_MASK)+
#define FS_INODE_IS_TYPE(mode, type) (((mode) & FS_INODE_TYPE_MASK) == (type))
#define FS_INODE_IS_SPECIAL(mode) (FS_INODE_IS_TYPE(mode, FS_INODE_BLOCK_SPECIAL) || FS_INODE_IS_TYPE(mode, FS_INODE_CHAR_SPECIAL))

enum
{
    FS_MAX_FILENAME = 12,
};
typedef struct dir_entry_t
{
	int	inode_index;
	char filename[FS_MAX_FILENAME];
} dir_entry_t;
enum
{
    FS_DIRENTRY_SIZE = sizeof(dir_entry_t),
    FS_DIRENTRY_PER_SECTOR = HD_SECTOR_SIZE / FS_DIRENTRY_SIZE,
};

enum
{
    FS_DEFAULT_FILE_SECTORS = 2048,
    FS_MAGIC_V1 = 0x111,
    FS_SUPER_BLOCK_MAGIC_V1 = 0x111,
    FS_SUPER_BLOCK_SIZE = 56, // Size on disk
    FS_INODE_SIZE = 32, // Size on disk
    FS_INODE_PER_SECTOR = HD_SECTOR_SIZE / FS_INODE_SIZE,
    FS_INODE_INVALID = 0,
    FS_INODE_ROOT = 1,
};

enum 
{
    FS_INVALID_DRIVER = -1,
};

void fs_read_sector(int device, int sector, void* buffer);
void fs_write_sector(int device, int sector, const void* buffer);
void fs_readwrite_sector(int type, int device, u64 position, int bytes, int proccess_index, void* buffer);

super_block_t* fs_get_super_block(int device);
inode_t* fs_get_inode(int device, int inode);
void fs_write_inode(inode_t* node);
void fs_put_inode(inode_t* node);
int fs_search_file(const char* path);
int fs_strip_path(char* filename, const char* filepath, inode_t** node);

void task_fs();

HEADER_CPP_END