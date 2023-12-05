#pragma once

#include <sys/const.h>
#include <sys/type.h>
#include <sys/proc.h>
#include <sys/sendrecv.h>

HEADER_CPP_BEGIN

enum
{
    SYSCALL_NUMINT = 0x90, // int 0x90
    SYSCALL_SENDRECV = 1,
    SYSCALL_WRITEX = 2,
    SYSCALL_COUNT
};

typedef void* system_call;
extern system_call sys_call_table[SYSCALL_COUNT];

u32 sys_sendrecv(u32 mode, u32 target, message_t* msg, process_t* process);

u32 sys_writex(const char* s, void*, void*, process_t* process);

HEADER_CPP_END