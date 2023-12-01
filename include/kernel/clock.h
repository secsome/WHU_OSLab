#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

// 8253/8254 PIT (Programmable Interval Timer)
enum 
{
	CLOCK_TIMER0 = 0x40,			// I/O port for timer channel 0
	CLOCK_TIMER_MODE = 0x43,		// I/O port for timer mode control
	CLOCK_RATE_GENERATOR = 0x34,	// 00-11-010-0, Counter0 - LSB then MSB - rate generator - binary
	CLOCK_TIMER_FREQ = 1193182L,	// clock frequency for timer in PC and AT
	CLOCK_HZ = 100					// clock freq (software settable on IBM-PC)
};

void init_clock();

extern u32 sys_tick_count;
void clock_handler(int irq);

HEADER_CPP_END