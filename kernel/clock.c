#include <kernel/clock.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <lib/display.h>

void clock_handler(int irq)
{
    ++sys_tick_count;
    
    if (p_proc_ready->ticks > 0)
    {
        --p_proc_ready->ticks;
        return;
    }

    if (k_reenter != 0)
        return;
    
    schedual();
}