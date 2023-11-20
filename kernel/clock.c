#include <kernel/clock.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <lib/display.h>

void clock_handler(int irq)
{
    ++sys_tick_count;

    if (k_reenter != 0)
        return;

    ++p_proc_ready;
    if (p_proc_ready >= proc_table + NUM_TASKS)
        p_proc_ready = proc_table;
}