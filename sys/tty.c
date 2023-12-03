#include <sys/tty.h>
#include <sys/keyboard.h>
#include <sys/console.h>
#include <sys/proc.h>
#include <lib/display.h>
#include <lib/asm.h>
#include <lib/assert.h>

tty_t ttys_table[NUM_CONSOLES];

static void tty_init(tty_t* tty);
static void tty_do_read(tty_t* tty);
static void tty_do_write(tty_t* tty);
static void tty_put_key(tty_t* tty, u32 key);

void task_tty()
{
    init_keyboard();

    for (tty_t* tty = ttys_table; tty < ttys_table + NUM_CONSOLES; ++tty)
        tty_init(tty);
    
    console_select(0);
    while (true)
    {
        for (tty_t* tty = ttys_table; tty < ttys_table + NUM_CONSOLES; ++tty)
        {
            tty_do_read(tty);
            tty_do_write(tty);
        }
    }
}

u32 tty_write(tty_t* tty, const char* buffer, u32 length)
{
    while (length--)
        console_put_char(tty->console, *buffer++);
    return 0;
}

void tty_process_input(tty_t* tty, u32 key)
{
    if (!(key & KEYBOARD_FLAG_EXT))
        tty_put_key(tty, key);
    else
    {
        const u32 raw_code = key & KEYBOARD_MASK_RAW;
        switch (raw_code)
        {
            case KEYBOARD_CODE_ENTER:
                tty_put_key(tty, '\n');
                break;
            case KEYBOARD_CODE_BACKSPACE:
                tty_put_key(tty, '\b');
                break;
            case KEYBOARD_CODE_UP:
                if ((key & KEYBOARD_FLAG_LSHIFT) || (key & KEYBOARD_FLAG_RSHIFT))
                    console_scroll(tty->console, false);
                break;
            case KEYBOARD_CODE_DOWN:
                if ((key & KEYBOARD_FLAG_LSHIFT) || (key & KEYBOARD_FLAG_RSHIFT))
                    console_scroll(tty->console, true);
                break;
            case KEYBOARD_CODE_F1:
            case KEYBOARD_CODE_F2:
            case KEYBOARD_CODE_F3:
            case KEYBOARD_CODE_F4:
            case KEYBOARD_CODE_F5:
            case KEYBOARD_CODE_F6:
            case KEYBOARD_CODE_F7:
            case KEYBOARD_CODE_F8:
            case KEYBOARD_CODE_F9:
            case KEYBOARD_CODE_F10:
            case KEYBOARD_CODE_F11:
            case KEYBOARD_CODE_F12:
                if ((key & KEYBOARD_FLAG_LALT) || (key & KEYBOARD_FLAG_RALT))
                    console_select(raw_code - KEYBOARD_CODE_F1);
                break;
            default:
                break;
        }
    }
}

void tty_init_screen(tty_t* tty)
{
    const int tty_index = tty - ttys_table;
    tty->console = consoles_table + tty_index;
	const u32 video_memory_size = V_MEM_SIZE / 2; // Video memory size in WORD

    const u32 video_memory_size_per_console = video_memory_size / NUM_CONSOLES;
	tty->console->original_address = tty_index * video_memory_size_per_console;
	tty->console->memory_limit = video_memory_size_per_console;
	tty->console->current_start_address = tty->console->original_address;
	tty->console->cursor = tty->console->original_address;

	if (tty_index == 0) 
    {
        extern int disp_pos;
		tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else
    {
		console_put_char(tty->console, tty_index + '0');
		console_put_char(tty->console, '#');
	}

    console_flush(tty->console);
}

static void tty_init(tty_t* tty)
{
    tty->input_buffer_count = 0;
    tty->input_buffer_head = tty->input_buffer_tail = tty->input_buffer;

    tty_init_screen(tty);
}

static void tty_do_read(tty_t* tty)
{
    if (console_is_current(tty->console))
        keyboard_read(tty);
}

static void tty_do_write(tty_t* tty)
{
    if (tty->input_buffer_count > 0) 
    {
		char ch = *tty->input_buffer_tail++;
		if (tty->input_buffer_tail == tty->input_buffer + TTY_INPUT_SIZE) {
			tty->input_buffer_tail = tty->input_buffer;
		}
		--tty->input_buffer_count;
		console_put_char(tty->console, ch);
	}
}

void tty_put_key(tty_t* tty, u32 key)
{
    if (tty->input_buffer_count < TTY_INPUT_SIZE)
    {
        *tty->input_buffer_head++ = key;
        if (tty->input_buffer_head == tty->input_buffer + TTY_INPUT_SIZE)
            tty->input_buffer_head = tty->input_buffer;
        ++tty->input_buffer_count;
    }
}

u32 writex_impl(const char* s, const process_t* process)
{
    enum
    {
        MAG_CH_PANIC = '\002',
        MAG_CH_ASSERT = '\003'
    };

    const char* p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	if (k_reenter == 0)  // writex() called in Ring<1~3>
		p = va2la(proc2pid(process), s);
	else if (k_reenter > 0) // writex() called in Ring<0>
		p = s;
	else // should never happen
		p = reenter_err;
	
	// if assertion fails in any TASK, the system will be halted;
	// if it fails in a USER PROC, it'll return like any normal syscall does.
	if ((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NUM_TASKS]))
    {
		disable_int();
		char* v = (char*)V_MEM_BASE;
		const char* q = p + 1; // +1: skip the magic char

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE))
        {
			*v++ = *q++;
			*v++ = COLOR_RED;
			if (!*q) {
				while (((int)v - V_MEM_BASE) % (CONSOLE_SCREEN_WIDTH * 16)) {
					/* *v++ = ' '; */
					v++;
					*v++ = COLOR_GRAY;
				}
				q = p + 1;
			}
		}

		halt();
	}

	while ((ch = *p++) != 0)
    {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue; // skip the magic char

		console_put_char(ttys_table[process->tty_index].console, ch);
	}

	return 0;
}