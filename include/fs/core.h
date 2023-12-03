#pragma once

#include <sys/const.h>
#include <sys/type.h>

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

typedef struct file_descriptor_t
{
    int mode;
    int position;
    inode_t* inode;
} file_descriptor_t;

enum
{
    FS_MAGIC_V1 = 0x111,
    FS_SUPER_BLOCK_MAGIC_V1 = 0x111,
    FS_SUPER_BLOCK_SIZE = 56, // Size on disk
    FS_INODE_SIZE = 32, // Size on disk

};

void read_sector();
void write_sector();
void readwrite_sector();

void task_fs();

HEADER_CPP_END