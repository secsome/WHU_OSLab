#include <sys/proc.h>
#include <sys/global.h>
#include <sys/clock.h>
#include <sys/keyboard.h>
#include <sys/tty.h>
#include <sys/syscall.h>
#include <sys/sendrecv.h>
#include <sys/harddisk.h>

#include <lib/asm.h>
#include <lib/syscall.h>
#include <lib/string.h>
#include <lib/printf.h>
#include <lib/display.h>
#include <lib/clock.h>
#include <lib/assert.h>

#include <fs/core.h>
#include <fs/fd.h>

void TestA();
void TestB();
void TestC();

void task_sys();

process_t* p_proc_ready;
process_t proc_table[NUM_TASKS + NUM_PROCS];
char task_stack[STACK_SIZE_TOTAL];
int k_reenter;
task_t system_task_table[NUM_TASKS] = 
{
	{
		task_tty,
		STACK_SIZE_TTY,
		"TTY"
	},
	{
		task_sys,
		STACK_SIZE_SYS,
		"SYS"
	},
	{
		task_harddisk,
		STACK_SIZE_HARDDISK,
		"HARDDISK"
	},
	{
		task_fs,
		STACK_SIZE_FILESYSTEM,
		"FILESYSTEM"
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
	for (;;)
		;
}

void TestB()
{	
    char tty_name[] = "/dev_tty1";
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
	while (true)
	{
		write(fd_stdout, "$ ", 2);
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if (!strcmp(rdbuf, "hello")	)
			write(fd_stdout, "hello world!\n", 13);
		else
		{
			if (rdbuf[0])
			{
				write(fd_stdout, "{", 1);
				write(fd_stdout, rdbuf, r);
				write(fd_stdout, "}\n", 2);
			}
		}
	}
	assert(false); // never arrive here.
}

void TestC()
{
    for (;;)
		;
}

extern void restart();
int kernel_main()
{
	task_t* task = system_task_table;
	process_t* proc = proc_table;
	char* current_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int priority;
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
			priority = 15;
		}
		else // User processes
		{
			task = user_proc_table + i - NUM_TASKS;
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; // IF=1, bit 2 is always 1
			priority = 5;
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

		proc->process_flags = 0;
		proc->msg = NULL;
		proc->recvfrom = SR_TARGET_NONE;
		proc->sendto = SR_TARGET_NONE;
		proc->has_int_msg = false;
		proc->sending = NULL;
		proc->next_sending = NULL;

		for (int j = 0; j < FS_NUM_FILES; ++j)
			proc->fd_table[j] = NULL;

		proc->ticks = proc->priority = priority;

		current_task_stack -= task->stacksize;
		proc++;
		task++;
		selector_ldt += 1 << 3;
	}
	
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
			if (p->process_flags == 0)
			{
				if (p->ticks > greatest_ticks)
				{
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}

		if (!greatest_ticks)
		{
			for (process_t* p = proc_table; p < proc_table + NUM_TASKS + NUM_PROCS; ++p)
			{
				if (p->process_flags == 0)
					p->ticks = p->priority;
			}
		}
	}
}

int process_get_linear_address(const process_t* p, int idx)
{
	const descriptor_t* desc = &p->ldts[idx];
	return desc->base_high << 24 | desc->base_mid << 16 | desc->base_low;
}

void* va2la(int pid, const void* virtual_address)
{
	const process_t* proc = &proc_table[pid];
	u32 linear_address =  process_get_linear_address(proc, LDT_RW) + (u32)virtual_address;

	if (pid < NUM_TASKS + NUM_PROCS)
		assert(linear_address == (u32)virtual_address);

	return (void*)linear_address;
}

void process_block(process_t* proc)
{
	assert(proc->process_flags);
	schedual();
}

void process_unblock(process_t* proc)
{
	assert(proc->process_flags == 0);
}

bool process_check_deadlock(int src, int dst)
{
	process_t* proc = proc_table + dst;
	while (true)
	{
		if (proc->process_flags & SR_STATUS_SENDING)
		{
			if (proc->sendto == src)
			{
				char buffer[1024] = "=_=";
				strcat(buffer, proc->process_name);
				do
				{
					assert(proc->msg);
					proc = proc_table + proc->sendto;
					strcat(buffer, "->");
					strcat(buffer, proc->process_name);
				} while (proc != proc_table + src);
				strcat(buffer, "=_=");

				return true;
			}	
			proc = proc_table + proc->sendto;
		}
		else
			break;
	}
	
	return false;
}

void process_inform_interrupt(int target)
{
	process_t* proc = proc_table + target;
	if ((proc->process_flags & SR_STATUS_RECEIVING) && 
		((proc->recvfrom == SR_TARGET_INTERRUPT) || (proc->recvfrom == SR_TARGET_ANY)))
	{
		proc->msg->source = SR_TARGET_INTERRUPT;
		proc->msg->type = SR_MSGTYPE_HARDINT;
		proc->msg = NULL;
		proc->has_int_msg = false;
		proc->process_flags &= ~SR_STATUS_RECEIVING;
		proc->recvfrom = SR_TARGET_NONE;
		assert(proc->process_flags == 0);
		process_unblock(proc);

		assert(proc->process_flags == 0);
		assert(proc->msg == NULL);
		assert(proc->recvfrom == SR_TARGET_NONE);
		assert(proc->sendto == SR_TARGET_NONE);
	}
	else
		proc->has_int_msg = true;
}

void process_init_io()
{
	assert(open("/dev_tty0", O_RDWR) == STDIN_FILENO);
	assert(open("/dev_tty1", O_RDWR) == STDOUT_FILENO);
	assert(open("/dev_tty2", O_RDWR) == STDERR_FILENO);
}