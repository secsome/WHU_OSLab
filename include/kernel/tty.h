// Teleprinter (TTY)
#pragma once

#include <kernel/type.h>

enum
{
    TTY_INPUT_SIZE = 256,
};

struct console_t;

typedef struct tty_t
{
    u32 input_buffer[TTY_INPUT_SIZE];
    u32* input_buffer_head;
    u32* input_buffer_tail;
    int input_buffer_count;
    struct console_t* console;
} tty_t;

extern tty_t ttys_table[];

void task_tty();

void tty_process_input(tty_t* tty, u32 key);
void tty_init_screen(tty_t* tty);