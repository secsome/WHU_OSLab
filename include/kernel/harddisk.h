#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

enum
{
    HD_SECTOR_SIZE = 512,
    HD_SECTOR_BITS = (HD_SECTOR_SIZE * 8),
    HD_SECTOR_SIZE_SHIFT = 9,
};

typedef struct partition_entry_t
{
    u8 boot_indicator;
    u8 starting_head;
    u8 starting_sector;
    u8 starting_cylinder;
    u8 system_id;
    u8 ending_head;
    u8 ending_sector;
    u8 ending_cylinder;
    u32 starting_sector32;
    u32 number_of_sectors32;
} partition_entry_t;

enum
{
    HD_REG_DATA = 0x1F0,
    HD_REG_FEATURES = 0x1F1,
    HD_REG_ERRORS = HD_REG_FEATURES,
    HD_REG_NSECTOR = 0x1F2,
    HD_REG_LBA_LOW = 0x1F3,
    HD_REG_LBA_MID = 0x1F4,
    HD_REG_LBA_HIGH = 0x1F5,
    HD_REG_DEVICE = 0x1F6,
    HD_REG_STATUS = 0x1F7,
    HD_REG_CMD = HD_REG_STATUS,
    HD_REG_DEV_CTRL = 0x3F6,
    HD_REG_ALT_STATUS = HD_REG_DEV_CTRL,
    HD_REG_DRV_ADDR = 0x3F7,

    HD_STATUS_BSY = 0x80,
    HD_STATUS_DRDY = 0x40,
    HD_STATUS_DFSE = 0x20,
    HD_STATUS_DSC = 0x10,
    HD_STATUS_DRQ = 0x08,
    HD_STATUS_CORR = 0x04,
    HD_STATUS_IDX = 0x02,
    HD_STATUS_ERR = 0x01,
};

typedef struct harddisk_command_t
{
    u8 features;
    u8 sector_count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 device;
    u8 command;
} harddisk_command_t;

typedef struct partition_info_t
{
    u32 base;
    u32 size;
} partition_info_t;

enum
{
    HD_MAX_DRIVES = 2,
    HD_NUM_PART_PER_DRIVE = 4,
    HD_NUM_SUB_PER_PART = 16,
    HD_NUM_SUB_PER_DRIVE = HD_NUM_SUB_PER_PART * HD_NUM_PART_PER_DRIVE,
    HD_NUM_PRIM_PER_DRIVE = HD_NUM_PART_PER_DRIVE + 1,
    HD_MAX_PRIM = HD_MAX_DRIVES * HD_NUM_PRIM_PER_DRIVE - 1,
    HD_MAX_SUBPARTITIONS = HD_NUM_SUB_PER_DRIVE * HD_MAX_DRIVES
};

typedef struct harddisk_info_t
{
    int open_count;
    partition_info_t primary[HD_NUM_PRIM_PER_DRIVE];
    partition_info_t logical[HD_MAX_SUBPARTITIONS];
} harddisk_info_t;

enum
{
    HD_TIMEOUT = 10000, // in millisec
    HD_PARTITION_TABLE_OFFSET = 0x1BE,
    HD_ATA_IDENTIFY = 0xEC,
    HD_ATA_READ = 0x20,
    HD_ATA_WRITE = 0x30,
    
};
#define MAKE_DEVICE_REG(lba, drv, lba_highest) (((lba) << 6) | ((drv) << 4) | (lba_highest & 0xF) | 0xA0)

void task_harddisk();

HEADER_CPP_END