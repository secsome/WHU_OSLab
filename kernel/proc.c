#include <kernel/proc.h>
#include <kernel/global.h>
#include <kernel/clock.h>

#include <lib/string.h>
#include <lib/display.h>

void TestA();
void TestB();
void TestC();

process_t* p_proc_ready;
process_t proc_table[NUM_TASKS];
char task_stack[STACK_SIZE_TOTAL];
int k_reenter;
task_t task_table[NUM_TASKS] = 
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

void delay(int time)
{
    for (int t = 0; t < time; ++t)
    {
        for (int i = 0; i < 10000000; ++i)
        {
        }
    }
}

void TestA()
{
    int i = 0;
    while (true)
    {
        disp_str(" ");
        disp_color_int(i++, 0xC);
        delay(1);
    }
}

void TestB()
{
    int i = 0;
    while (true)
    {
        disp_str(" ");
        disp_color_int(i++, 0xD);
        delay(1);
    }
}

void TestC()
{
    int i = 0;
    while (true)
    {
        disp_str(" ");
        disp_color_int(i++, 0xE);
        delay(1);
    }
}

extern void restart();
int kernel_main()
{
    disp_str("-----\"kernel_main\" begins-----\n");

	task_t* p_task = task_table;
	process_t* p_proc = proc_table;
	char* p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	for (int i = 0; i < NUM_TASKS; ++i)
	{
		strncpy(p_proc->process_name, p_task->name, 16); // name of the process
		p_proc->pid = i; // pid
		p_proc->ldt_selector = selector_ldt;
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(descriptor_t));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(descriptor_t));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	k_reenter = 0;

	p_proc_ready = proc_table; 

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);

	restart();

	while (true)
	{
		// Do nothing
	}
}