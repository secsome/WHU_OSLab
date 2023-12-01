#include <kernel/clock.h>
#include <kernel/proc.h>
#include <lib/asm.h>
#include <lib/display.h>

void init_clock()
{
    out_byte(CLOCK_TIMER_MODE, CLOCK_RATE_GENERATOR);
    out_byte(CLOCK_TIMER0, (u8)(CLOCK_TIMER_FREQ / CLOCK_HZ));
    out_byte(CLOCK_TIMER0, (u8)((CLOCK_TIMER_FREQ / CLOCK_HZ) >> 8));
	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);
}

u32 sys_tick_count = 0;
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