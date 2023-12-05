#pragma once

#include <sys/const.h>

#include <fs/config.h>

HEADER_CPP_BEGIN

struct inode_t;
typedef struct file_descriptor_t
{
    int mode;
    int position;
    struct inode_t* inode;
} file_descriptor_t;
extern file_descriptor_t fs_file_descriptor_table[FS_NUM_FD];

enum
{
    FS_FDFLAGS_CREAT = 1,
    FS_FDFLAGS_RDWR = 2,

    O_CREAT = FS_FDFLAGS_CREAT,
    O_RDWR = FS_FDFLAGS_RDWR,
};

int open(const char* path, int flags);
int close(int fd);
int read(int fd, void* buf, size_t count);
int write(int fd, const void* buf, size_t count);

HEADER_CPP_END