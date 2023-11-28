#include <kernel/console.h>
#include <lib/asm.h>
#include <lib/display.h>

console_t consoles_table[NUM_CONSOLES];

u32 current_console_id;

bool is_current_console(console_t* console)
{
    return console == &consoles_table[current_console_id];
}

extern int disp_pos;
void console_putchar(console_t* console, char ch)
{
    u8* vmem = (u8*)(V_MEM_BASE + disp_pos);
    *vmem++ = ch;
    *vmem++ = COLOR_WHITE;
    disp_pos += 2;
    console_setcursor(disp_pos / 2);
}

void console_setcursor(u32 position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}