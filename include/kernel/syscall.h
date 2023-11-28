#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/proc.h>

enum
{
    SYSCALL_NUMINT = 0x90, // int 0x90
    SYSCALL_GETTICKS = 0,
    SYSCALL_WRITE = 1,
    SYSCALL_COUNT
};

typedef void* system_call;
extern system_call sys_call_table[SYSCALL_COUNT];

extern u32 sys_tick_count;
u32 sys_get_ticks();

u32 sys_write(const char* buffer, u32 length, const process_t* process);