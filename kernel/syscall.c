#include <kernel/syscall.h>
#include <kernel/tty.h>

system_call sys_call_table[SYSCALL_COUNT] = 
{
    sys_get_ticks,
    sys_write,
};

u32 sys_tick_count;
u32 sys_get_ticks()
{
    return sys_tick_count;
}

u32 sys_write(const char* buffer, u32 length, const process_t* process)
{
    return tty_write(&ttys_table[process->tty_index], buffer, length);
}