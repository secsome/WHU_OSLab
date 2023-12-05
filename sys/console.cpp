#include <sys/console.h>
#include <lib/asm.h>
#include <lib/string.h>
#include <lib/assert.h>

console_t consoles_table[NUM_CONSOLES];

u32 current_console_id;

static void console_clear_screen(u32 position, u32 length);
static void console_set_cursor(u32 position);
static void console_set_video_start_address(u32 address);

bool console_is_current(console_t* console)
{
    return console == &consoles_table[current_console_id];
}

void console_put_char(console_t* console, char ch)
{
    u8* pch = (u8*)(V_MEM_BASE + console->cursor * 2);

	assert(console->cursor - console->original_address < console->console_size);

	// calculate the coordinate of cursor in current console (not in current screen)
	int cursor_x = (console->cursor - console->original_address) % CONSOLE_SCREEN_WIDTH;
	int cursor_y = (console->cursor - console->original_address) / CONSOLE_SCREEN_WIDTH;

	switch(ch)
    {
	case '\n':
		console->cursor = console->original_address + CONSOLE_SCREEN_WIDTH * (cursor_y + 1);
		break;
	case '\b':
		if (console->cursor > console->original_address)
        {
			--console->cursor;
			*(pch - 2) = ' ';
			*(pch - 1) = MAKE_COLOR(COLOR_BLACK, COLOR_WHITE);
		}
		break;
	default:
		*pch++ = ch;
		*pch++ = MAKE_COLOR(COLOR_BLACK, COLOR_WHITE);
		++console->cursor;
		break;
	}

	if (console->cursor - console->original_address >= console->console_size)
    {
		cursor_x = (console->cursor - console->original_address) % CONSOLE_SCREEN_WIDTH;
		cursor_y = (console->cursor - console->original_address) / CONSOLE_SCREEN_WIDTH;
		u32 cp_original = console->original_address + (cursor_y + 1) * CONSOLE_SCREEN_WIDTH - CONSOLE_SCREEN_SIZE;
		memcpy(
            reinterpret_cast<char*>(V_MEM_BASE + console->original_address * 2), 
            reinterpret_cast<const char*>(V_MEM_BASE + cp_original * 2), 
            (CONSOLE_SCREEN_SIZE - CONSOLE_SCREEN_WIDTH) * 2);
		console->current_start_address = console->original_address;
		console->cursor = console->original_address + (CONSOLE_SCREEN_SIZE - CONSOLE_SCREEN_WIDTH) + cursor_x;
		console_clear_screen(console->cursor, CONSOLE_SCREEN_WIDTH);
		if (!console->is_full)
			console->is_full = true;
	}

	assert(console->cursor - console->original_address < console->console_size);

	while (
        console->cursor >= console->current_start_address + CONSOLE_SCREEN_SIZE || 
        console->cursor < console->current_start_address) 
    {
		console_scroll(console, true);
		console_clear_screen(console->cursor, CONSOLE_SCREEN_WIDTH);
	}

    console_flush(console);
}

void console_flush(console_t* console)
{
    if (console_is_current(console))
    {
        console_set_cursor(console->cursor);
        console_set_video_start_address(console->current_start_address);
    }
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
	// variables below are all in-console-offsets (based on console->original_address)
	
	// addr of the newest available line in the console
	auto newest = (console->cursor - console->original_address) / CONSOLE_SCREEN_WIDTH * CONSOLE_SCREEN_WIDTH;
	// addr of the oldest available line in the console
	auto oldest = console->is_full ? (newest + CONSOLE_SCREEN_WIDTH) % console->console_size : 0;
	// position of the top of current screen
	auto screen_top = console->current_start_address - console->original_address;

	if (up)
	{
		if (!console->is_full && newest >= screen_top + CONSOLE_SCREEN_SIZE)
			console->current_start_address += CONSOLE_SCREEN_WIDTH;
		else if (console->is_full && screen_top + CONSOLE_SCREEN_SIZE - CONSOLE_SCREEN_WIDTH != newest)
		{
			if (screen_top + CONSOLE_SCREEN_SIZE == console->console_size)
				console->current_start_address = console->original_address;
			else
				console->current_start_address += CONSOLE_SCREEN_WIDTH;
		}
	}
	else
	{
		if (!console->is_full && screen_top > 0)
			console->current_start_address -= CONSOLE_SCREEN_WIDTH;
		else if (console->is_full && screen_top != oldest)
		{
			if (console->cursor - console->original_address >= console->console_size - CONSOLE_SCREEN_SIZE)
			{
				if (console->current_start_address != console->original_address)
					console->current_start_address -= CONSOLE_SCREEN_WIDTH;
			}
			else if (console->current_start_address == console->original_address)
			{
				screen_top = console->console_size - CONSOLE_SCREEN_SIZE;
				console->current_start_address = console->original_address + screen_top;
			}
			else
				console->current_start_address -= CONSOLE_SCREEN_WIDTH;
		}
	}

	console_flush(console);
}

static void console_clear_screen(u32 position, u32 length)
{
    u8* pch = (u8*)(V_MEM_BASE + position * 2);
	for (u32 i = 0; i < length; ++i)
    {
        *pch++ = ' ';
        *pch++ = MAKE_COLOR(COLOR_BLACK, COLOR_WHITE);
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