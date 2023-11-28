#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <kernel/console.h>
#include <lib/display.h>
#include <lib/asm.h>

tty_t ttys_table[NUM_CONSOLES];

static void tty_init(tty_t* tty);
static void tty_do_read(tty_t* tty);
static void tty_do_write(tty_t* tty);

void task_tty()
{
    init_keyboard();

    for (tty_t* tty = ttys_table; tty < ttys_table + NUM_CONSOLES; ++tty)
        tty_init(tty);
    
    current_console_id = 0;
    while (true)
    {
        for (tty_t* tty = ttys_table; tty < ttys_table + NUM_CONSOLES; ++tty)
        {
            tty_do_read(tty);
            tty_do_write(tty);
        }
    }
}

void in_process(tty_t* tty, u32 key)
{
    char output[2] = { 0 };
    if (!(key & KEYBOARD_FLAG_EXT))
    {
        if (tty->input_buffer_count < TTY_INPUT_SIZE)
        {
            *tty->input_buffer_head++ = key;
            if (tty->input_buffer_head == tty->input_buffer + TTY_INPUT_SIZE)
                tty->input_buffer_head = tty->input_buffer;
            ++tty->input_buffer_count;
        }
    }
    else
    {
        switch (key & KEYBOARD_MASK_RAW)
        {
            case KEYBOARD_CODE_UP:
                if ((key & KEYBOARD_FLAG_LSHIFT) || (key & KEYBOARD_FLAG_RSHIFT))
                {
                    disable_int();
                    out_byte(CRTC_ADDR_REG, START_ADDR_H);
                    out_byte(CRTC_DATA_REG, ((80 * 15) >> 8) & 0xFF);
                    out_byte(CRTC_ADDR_REG, START_ADDR_L);
                    out_byte(CRTC_DATA_REG, (80 * 15) & 0xFF);
                    enable_int();
                }
            case KEYBOARD_CODE_DOWN:
                if ((key & KEYBOARD_FLAG_LSHIFT) || (key & KEYBOARD_FLAG_RSHIFT))
                {
                    // Do nothing now
                }
                break;
            default:
                break;
        }
    }
}

static void tty_init(tty_t* tty)
{
    tty->input_buffer_count = 0;
    tty->input_buffer_head = tty->input_buffer_tail = tty->input_buffer;

    const int tty_index = tty - ttys_table;
    tty->console = consoles_table + tty_index;
}

static void tty_do_read(tty_t* tty)
{
    if (is_current_console(tty->console))
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
		console_putchar(tty->console, ch);
	}
}