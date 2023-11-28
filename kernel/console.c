#include <kernel/console.h>
#include <lib/asm.h>

console_t consoles_table[NUM_CONSOLES];

u32 current_console_id;

static void console_set_cursor(u32 position);
static void console_set_video_start_address(u32 address);

bool console_is_current(console_t* console)
{
    return console == &consoles_table[current_console_id];
}

void console_put_char(console_t* console, char ch)
{
    u8* vmem = (u8*)(V_MEM_BASE + console->cursor * 2);

	switch (ch) 
    {
	case '\n':
		if (console->cursor < console->original_address + console->memory_limit - CONSOLE_SCREEN_WIDTH) 
			console->cursor = 
                console->original_address + 
                CONSOLE_SCREEN_WIDTH * ((console->cursor - console->original_address) / CONSOLE_SCREEN_WIDTH + 1);
		break;
	case '\b':
		if (console->cursor > console->original_address) 
        {
			--console->cursor;
			*(vmem - 2) = ' ';
			*(vmem - 1) = COLOR_WHITE;
		}
		break;
	default:
		if (console->cursor < console->original_address + console->memory_limit - 1) 
        {
			*vmem++ = ch;
			*vmem++ = COLOR_WHITE;
			++console->cursor;
		}
		break;
	}

	while (console->cursor >= console->current_start_address + CONSOLE_SCREEN_SIZE)
		console_scroll(console, false);

	console_flush(console);
}

void console_flush(console_t* console)
{
    console_set_cursor(console->cursor);
    console_set_video_start_address(console->current_start_address);
}

void console_select(u32 console_id)
{
    if (console_id >= NUM_CONSOLES)
        return;
    
    current_console_id = console_id;
    console_set_cursor(consoles_table[current_console_id].cursor);
    console_set_video_start_address(consoles_table[current_console_id].current_start_address);
}

void console_scroll(console_t* console, bool up)
{
    if (up)
    {
        if (console->current_start_address > console->original_address)
        {
            console->current_start_address -= CONSOLE_SCREEN_WIDTH;
            console_set_cursor(console->cursor);
            console_set_video_start_address(console->current_start_address);
        }
    }
    else
    {
        if (console->current_start_address + CONSOLE_SCREEN_SIZE < 
            console->original_address + console->memory_limit)
        {
            console->current_start_address += CONSOLE_SCREEN_WIDTH;
            console_set_cursor(console->cursor);
            console_set_video_start_address(console->current_start_address);
        }
    }
}

static void console_set_cursor(u32 position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

static void console_set_video_start_address(u32 address)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (address >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, address & 0xFF);
    enable_int();
}