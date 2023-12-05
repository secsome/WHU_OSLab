#pragma once

#include <sys/const.h>

#include <fs/config.h>

HEADER_CPP_BEGIN

#ifndef TYPE_OFF_T_DEFINED
    #define TYPE_OFF_T_DEFINED
    typedef int off_t;
#endif // TYPE_OFF_T_DEFINED

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
    FS_FD_STDIN = 0,
    FS_FD_STDOUT = 1,
    FS_FD_STDERR = 2,
    FS_FDFLAGS_CREAT = 1,
    FS_FDFLAGS_RDWR = 2,
    FS_LSEEK_SET = 0,
    FS_LSEEK_CUR = 1,
    FS_LSEEK_END = 2,

    STDIN_FILENO = FS_FD_STDIN,
    STDOUT_FILENO = FS_FD_STDOUT,
    STDERR_FILENO = FS_FD_STDERR,

    O_CREAT = FS_FDFLAGS_CREAT,
    O_RDWR = FS_FDFLAGS_RDWR,

    SEEK_SET = FS_LSEEK_SET,
    SEEK_CUR = FS_LSEEK_CUR,
    SEEK_END = FS_LSEEK_END,
};

int open(const char* path, int flags);
int close(int fd);
int read(int fd, void* buf, size_t count);
int write(int fd, const void* buf, size_t count);
int unlink(const char* pathname);
int stat(const char* pathname);
off_t lseek(int fd, off_t offset, int whence);

int creat(const char* pathname);
int remove(const char* pathname);


HEADER_CPP_END