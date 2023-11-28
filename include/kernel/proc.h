#pragma once

#include <kernel/type.h>
#include <kernel/protect.h>
#include <kernel/const.h>

typedef struct stackframe_t
{
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 return_address;
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
} stackframe_t;

typedef struct process_t
{
    stackframe_t regs;
    u16 ldt_selector;
    descriptor_t ldts[LDT_SIZE];
    u32 ticks; // remained ticks
    u32 priority;
    u32 pid;
    char process_name[16];
} process_t;

extern process_t* p_proc_ready;
enum
{
    NUM_TASKS = 1,
    NUM_PROCS = 3
};
extern process_t proc_table[NUM_TASKS + NUM_PROCS];

enum
{
    STACK_SIZE_TTY = 0x8000,
    STACK_SIZE_TESTA = 0x8000,
    STACK_SIZE_TESTB = 0x8000,
    STACK_SIZE_TESTC = 0x8000,
    STACK_SIZE_TOTAL = STACK_SIZE_TTY + STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC
};
extern char task_stack[STACK_SIZE_TOTAL];

extern int k_reenter;

typedef struct task_t
{
    farproc_t initial_eip;
    int stacksize;
    char name[32];
} task_t;

extern task_t system_task_table[NUM_TASKS];
extern task_t user_proc_table[NUM_PROCS];

void schedual();