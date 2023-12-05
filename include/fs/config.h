#pragma once

#include <sys/const.h>
#include <sys/harddisk.h>

HEADER_CPP_BEGIN

enum
{
    FS_NUM_FILES = 64,
    FS_NUM_FD = 64, // FIXME
    FS_NUM_INODE = 64, // FIXME
    FS_NUM_SUPERBLOCK = 8,
    FS_MAX_PATH = 260,
    MAX_PATH = FS_MAX_PATH
};

enum
{
    FS_MINOR_BOOT = HD_MINOR_2A
};

HEADER_CPP_END