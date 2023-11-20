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
    u32 pid;
    char process_name[16];
} process_t;

extern process_t* p_proc_ready;
#define TASKCNT_MAX	3
extern process_t proc_table[TASKCNT_MAX];

#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TOTAL STACK_SIZE_TESTA
extern char task_stack[STACK_SIZE_TOTAL];

extern int k_reenter;