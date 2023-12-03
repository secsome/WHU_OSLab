#include <sys/syscall.h>
#include <sys/tty.h>

system_call sys_call_table[SYSCALL_COUNT] = 
{
    sys_write,
    sys_sendrecv,
    sys_writex,
};

u32 sys_write(const char* buffer, u32 length, void*, process_t* process)
{
    return tty_write(&ttys_table[process->tty_index], buffer, length);
}

u32 sys_sendrecv(u32 mode, u32 target, message_t* msg, process_t* process)
{
    return sendrecv_impl(mode, target, msg, process);
}

extern u32 writex_impl(const char* s, const process_t* process);
u32 sys_writex(const char* s, void*, void*, process_t* process)
{
    return writex_impl(s, process);
}