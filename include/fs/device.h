#pragma once

#include <sys/const.h>
#include <fs/core.h>
#include <fs/config.h>

HEADER_CPP_BEGIN

enum
{
    FS_DEV_NONE = 0,
    FS_DEV_FLOPPY = 1,
    FS_DEV_CDROM = 2,
    FS_DEV_HARDDISK = 3,
    FS_DEV_CHAR_TTY = 4,
    FS_DEV_SCSI = 5,
};

enum
{
    FS_MAJOR_DEV_SHIFT = 8
};
#define MAKE_DEV(major, minor) ((major) << FS_MAJOR_DEV_SHIFT | (minor))
#define MAJOR_DEV(dev) ((dev) >> FS_MAJOR_DEV_SHIFT)
#define MINOR_DEV(dev) ((dev) & 0xFF)

#define GET_DEV_DRIVER(dev) (dev <= HD_MAX_PRIM ? dev / HD_NUM_PRIM_PER_DRIVE : (dev - HD_MINOR_1A) / HD_NUM_SUB_PER_DRIVE)

enum
{
    FS_ROOT_DEVICE = MAKE_DEV(FS_DEV_HARDDISK, FS_MINOR_BOOT),
};

extern dev_driver_map_t device_driver_map[];

HEADER_CPP_END