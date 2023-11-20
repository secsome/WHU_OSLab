#pragma once

#include <kernel/const.h>

enum
{
    SYSCALL_NUMINT = 0x90, // int 0x90
    SYSCALL_GETTICKS = 0,
    SYSCALL_COUNT
};

typedef void* system_call;
extern system_call sys_call_table[SYSCALL_COUNT];

extern int sys_tick_count;
int sys_get_ticks();