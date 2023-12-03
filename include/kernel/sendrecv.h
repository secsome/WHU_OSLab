#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/proc.h>

HEADER_CPP_BEGIN

struct _message_t1
{
    int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};

struct _message_t2
{
    int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};

struct _message_t3
{
    int	m3i1;
	int	m3i2;
	int	m3i3;
	int	m3i4;
	u64	m3l1;
	u64	m3l2;
	void* m3p1;
	void* m3p2;
};

typedef struct message_t
{
    int source;
    int type;
    union
    {
        struct _message_t1 m1;
        struct _message_t2 m2;
        struct _message_t3 m3;
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

    SR_MSGTYPE_DEVOPEN = 1001,
};

u32 sendrecv_impl(u32 mode, u32 target, message_t* msg, process_t* process);
u32 sendrecv(u32 mode, u32 target, message_t* msg);

HEADER_CPP_END