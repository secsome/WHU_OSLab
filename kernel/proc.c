#include <kernel/proc.h>
#include <kernel/global.h>
#include <kernel/clock.h>
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <kernel/syscall.h>

#include <lib/asm.h>
#include <lib/syscall.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <lib/display.h>
#include <lib/clock.h>

void TestA();
void TestB();
void TestC();

process_t* p_proc_ready;
process_t proc_table[NUM_TASKS + NUM_PROCS];
char task_stack[STACK_SIZE_TOTAL];
int k_reenter;
task_t system_task_table[NUM_TASKS] = 
{
	{
		task_tty,
		STACK_SIZE_TTY,
		"Teleprinter"
	}
};
task_t user_proc_table[NUM_PROCS] = 
{
	{
		TestA,
		STACK_SIZE_TESTA,
		"TestA"
	},
	{
		TestB,
		STACK_SIZE_TESTB,
		"TestB"
	},
	{
		TestC,
		STACK_SIZE_TESTC,
		"TestC"
	}
};



void TestA()
{
    while (true)
    {
		printf("<Ticks:%x>", lib_get_ticks());
		usleep(20000);
    }
}

void TestB()
{
    while (true)
    {
		printf("B");
        usleep(20000);
    }
}

void TestC()
{
    while (true)
    {
		printf("C");
        usleep(20000);
    }
}

extern void restart();
int kernel_main()
{
	task_t* task = system_task_table;
	process_t* proc = proc_table;
	char* current_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	for (int i = 0; i < NUM_TASKS + NUM_PROCS; ++i)
	{
		u8 privilege;
		u8 rpl;
		u32 eflags;
		if (i < NUM_TASKS) // System tasks
		{
			task = system_task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; // IF=1, IOPL=1, bit 2 is always 1
		}
		else // User processes
		{
			task = user_proc_table + i - NUM_TASKS;
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; // IF=1, bit 2 is always 1
		}
		strncpy(proc->process_name, task->name, 16); // name of the process
		proc->pid = i; // pid
		proc->ldt_selector = selector_ldt;
		memcpy(&proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(descriptor_t));
		proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(descriptor_t));
		proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		proc->regs.eip = (u32)task->initial_eip;
		proc->regs.esp = (u32)current_task_stack;
		proc->regs.eflags = eflags;
		proc->tty_index = 0;

		current_task_stack -= task->stacksize;
		proc++;
		task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 5;
	proc_table[2].ticks = proc_table[2].priority = 3;
	proc_table[3].ticks = proc_table[3].priority = 1;
	
	proc_table[1].tty_index = 0;
	proc_table[2].tty_index = 1;
	proc_table[3].tty_index = 1;

	k_reenter = 0;
	sys_tick_count = 0;

	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	disp_clear();
	disp_reset();

	restart();

	while (true)
	{
		// Do nothing
	}
}

void schedual()
{
	int greatest_ticks = 0;

	while (!greatest_ticks) 
	{
		for (process_t* p = proc_table; p < proc_table + NUM_TASKS + NUM_PROCS; ++p)
		{
			if (p->ticks > greatest_ticks)
			{
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks)
		{
			for (process_t* p = proc_table; p < proc_table + NUM_TASKS + NUM_PROCS; ++p)
				p->ticks = p->priority;
		}
	}
}