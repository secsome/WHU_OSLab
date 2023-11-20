#include <kernel/syscall.h>

system_call sys_call_table[SYSCALL_COUNT] = 
{
    sys_get_ticks,
};

int sys_tick_count;
int sys_get_ticks()
{
    return sys_tick_count;
}