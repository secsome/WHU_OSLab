#pragma once

#include <kernel/type.h>

typedef struct console_t
{
    u32 current_start_address; // where is current display starts
    u32 original_address; // where does this console's video memory starts
    u32 memory_limit; // console video memory limit
    u32 cursor; // current cursor position
} console_t;

enum
{
    NUM_CONSOLES = 3,
};

extern console_t consoles_table[];
extern u32 current_console_id;

enum
{
    CRTC_ADDR_REG = 0x3D4, // CRT Controller Registers - Addr Register
    CRTC_DATA_REG = 0x3D5, // CRT Controller Registers - Data Register
    START_ADDR_H = 0xC, // reg index of video mem start addr (MSB)
    START_ADDR_L = 0xD, // reg index of video mem start addr (LSB)
    CURSOR_H = 0xE, // reg index of cursor position (MSB)
    CURSOR_L = 0xF, // reg index of cursor position (LSB)
    V_MEM_BASE = 0xB8000, // base of color video memory
    V_MEM_SIZE = 0x8000, // 32K: B8000H -> BFFFFH
};

enum
{
    COLOR_BLACK = 0x0,    /* 0000 */
    COLOR_WHITE = 0x7,    /* 0111 */
    COLOR_RED = 0x4,      /* 0100 */
    COLOR_GREEN = 0x2,    /* 0010 */
    COLOR_BLUE = 0x1,     /* 0001 */
    COLOR_FLASH = 0x80,   /* 1000 0000 */
    COLOR_BRIGHT = 0x08,  /* 0000 1000 */
};

bool is_current_console(console_t* console);
void console_putchar(console_t* console, char ch);
void console_setcursor(u32 position);