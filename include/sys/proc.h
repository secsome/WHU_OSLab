#pragma once

#include <sys/const.h>
#include <sys/type.h>
#include <sys/protect.h>

#include <fs/core.h>
#include <fs/fd.h>

HEADER_CPP_BEGIN

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

struct message_t;

typedef struct process_t
{
    stackframe_t regs;
    u16 ldt_selector;
    descriptor_t ldts[LDT_COUNT];
    u32 ticks; // remained ticks
    u32 priority;
    u32 pid;
    char process_name[16];
    int process_flags;
    struct message_t* msg;
    int recvfrom;
    int sendto;
    bool has_int_msg;
    struct process_t* sending;
    struct process_t* next_sending;
    file_descriptor_t* fd_table[FS_NUM_FILES];
    
} process_t;

extern process_t* p_proc_ready;
enum
{
    NUM_TASKS = 4,
    NUM_PROCS = 3,   
};

enum 
{
    TASK_TTY = 0,
    TASK_SYS = 1,
    TASK_HARDDISK = 2,
    TASK_FILESYSTEM = 3,
};
extern process_t proc_table[NUM_TASKS + NUM_PROCS];
#define proc2pid(x) ((int)(x - proc_table))
int process_get_linear_address(const process_t* p, int idx);
void* va2la(int pid, const void* virtual_address);
void process_block(process_t* proc);
void process_unblock(process_t* proc);
bool process_check_deadlock(int src, int dst);
void process_inform_interrupt(int target);
void process_init_io();

enum
{
    STACK_SIZE_TTY = 0x8000,
    STACK_SIZE_SYS = 0x8000,
    STACK_SIZE_HARDDISK = 0x8000,
    STACK_SIZE_FILESYSTEM = 0x8000,
    STACK_SIZE_TESTA = 0x8000,
    STACK_SIZE_TESTB = 0x8000,
    STACK_SIZE_TESTC = 0x8000,
    STACK_SIZE_TOTAL = STACK_SIZE_TTY + STACK_SIZE_SYS + STACK_SIZE_HARDDISK + STACK_SIZE_FILESYSTEM +
        STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC
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

HEADER_CPP_END