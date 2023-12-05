#pragma once

#include <sys/const.h>
#include <sys/type.h>
#include <sys/proc.h>

HEADER_CPP_BEGIN

struct _message_device
{
    int device;
    union
    {
        int count;
        int request;
    };
    int process_index;
    u64 position;
    void* buffer;
};

struct _message_open
{
    const char* path;
    size_t path_length;
    int flags;
};

typedef struct message_t
{
    int source;
    int type;
    union
    {
        char m_raw[1];
        int m_int32;
        struct _message_device m_device;
        struct _message_open m_open;
    };
} message_t;

enum
{
    SR_STATUS_SENDING = 0x2,
    SR_STATUS_RECEIVING = 0x4,
    SR_MODE_SEND = 1,
    SR_MODE_RECEIVE = 2,
    SR_MODE_BOTH = 3,
    SR_TARGET_ANY = -1,
    SR_TARGET_INTERRUPT = -2,
    SR_TARGET_NONE = -3,
};

enum
{
    SR_MSGTYPE_HARDINT = 1,
    SR_MSGTYPE_GET_TICKS = 2,

    SR_MSGTYPE_OPEN,
    SR_MSGTYPE_CLOSE,
    SR_MSGTYPE_READ,
    SR_MSGTYPE_WRITE,
    SR_MSGTYPE_LSEEK,
    SR_MSGTYPE_STAT,
    SR_MSGTYPE_UNLINK,

    SR_MSGTYPE_SYSCALL_RET,

    SR_MSGTYPE_DEVOPEN = 1001,
    SR_MSGTYPE_DEVCLOSE,
    SR_MSGTYPE_DEVREAD,
    SR_MSGTYPE_DEVWRITE,
    SR_MSGTYPE_DEVIOCTL,
};

u32 sendrecv_impl(u32 mode, u32 target, message_t* msg, process_t* process);
u32 sendrecv(u32 mode, u32 target, message_t* msg);

HEADER_CPP_END