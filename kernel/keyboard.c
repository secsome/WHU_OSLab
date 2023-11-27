#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <kernel/protect.h>
#include <lib/display.h>
#include <lib/asm.h>

static keyboard_t keyboard_in;

static bool code_with_e0, lshift, rshift, lalt, ralt, lctrl, rctrl;
// static bool capslock, numlock, scrolllock;
static int column;

u32 keymap[KEYBOARD_NUM_SCAN_CODES * KEYBOARD_MAP_COLS] = {

/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,		0,		0,
/* 0x01 - ESC		*/	KEYBOARD_CODE_ESC,		KEYBOARD_CODE_ESC,		0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	KEYBOARD_CODE_BACKSPACE,	KEYBOARD_CODE_BACKSPACE,	0,
/* 0x0F - TAB		*/	KEYBOARD_CODE_TAB,		KEYBOARD_CODE_TAB,		0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	KEYBOARD_CODE_ENTER,		KEYBOARD_CODE_ENTER,		KEYBOARD_CODE_NUMPAD_ENTER,
/* 0x1D - l. Ctrl	*/	KEYBOARD_CODE_LCTRL,		KEYBOARD_CODE_LCTRL,		KEYBOARD_CODE_RCTRL,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	KEYBOARD_CODE_LSHIFT,	KEYBOARD_CODE_LSHIFT,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		KEYBOARD_CODE_NUMPAD_SLASH,
/* 0x36 - r. SHIFT	*/	KEYBOARD_CODE_RSHIFT,	KEYBOARD_CODE_RSHIFT,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	KEYBOARD_CODE_LALT,		KEYBOARD_CODE_LALT,  	KEYBOARD_CODE_RALT,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - CapsLock	*/	KEYBOARD_CODE_CAPSLOCK,	KEYBOARD_CODE_CAPSLOCK,	0,
/* 0x3B - F1		*/	KEYBOARD_CODE_F1,		KEYBOARD_CODE_F1,		0,
/* 0x3C - F2		*/	KEYBOARD_CODE_F2,		KEYBOARD_CODE_F2,		0,
/* 0x3D - F3		*/	KEYBOARD_CODE_F3,		KEYBOARD_CODE_F3,		0,
/* 0x3E - F4		*/	KEYBOARD_CODE_F4,		KEYBOARD_CODE_F4,		0,
/* 0x3F - F5		*/	KEYBOARD_CODE_F5,		KEYBOARD_CODE_F5,		0,
/* 0x40 - F6		*/	KEYBOARD_CODE_F6,		KEYBOARD_CODE_F6,		0,
/* 0x41 - F7		*/	KEYBOARD_CODE_F7,		KEYBOARD_CODE_F7,		0,
/* 0x42 - F8		*/	KEYBOARD_CODE_F8,		KEYBOARD_CODE_F8,		0,
/* 0x43 - F9		*/	KEYBOARD_CODE_F9,		KEYBOARD_CODE_F9,		0,
/* 0x44 - F10		*/	KEYBOARD_CODE_F10,		KEYBOARD_CODE_F10,		0,
/* 0x45 - NumLock	*/	KEYBOARD_CODE_NUMLOCK,	KEYBOARD_CODE_NUMLOCK,	0,
/* 0x46 - ScrLock	*/	KEYBOARD_CODE_SCROLLLOCK,	KEYBOARD_CODE_SCROLLLOCK,	0,
/* 0x47 - Home		*/	KEYBOARD_CODE_NUMPAD_HOME,	'7',		KEYBOARD_CODE_HOME,
/* 0x48 - CurUp		*/	KEYBOARD_CODE_NUMPAD_UP,		'8',		KEYBOARD_CODE_UP,
/* 0x49 - PgUp		*/	KEYBOARD_CODE_NUMPAD_PAGEUP,	'9',		KEYBOARD_CODE_PAGEUP,
/* 0x4A - '-'		*/	KEYBOARD_CODE_NUMPAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	KEYBOARD_CODE_NUMPAD_LEFT,	'4',		KEYBOARD_CODE_LEFT,
/* 0x4C - MID		*/	KEYBOARD_CODE_NUMPAD_MIDDLE,	'5',		0,
/* 0x4D - Right		*/	KEYBOARD_CODE_NUMPAD_RIGHT,	'6',		KEYBOARD_CODE_RIGHT,
/* 0x4E - '+'		*/	KEYBOARD_CODE_NUMPAD_PLUS,	'+',		0,
/* 0x4F - End		*/	KEYBOARD_CODE_NUMPAD_END,	'1',		KEYBOARD_CODE_END,
/* 0x50 - Down		*/	KEYBOARD_CODE_NUMPAD_DOWN,	'2',		KEYBOARD_CODE_DOWN,
/* 0x51 - PgDown	*/	KEYBOARD_CODE_NUMPAD_PAGEDOWN,	'3',		KEYBOARD_CODE_PAGEDOWN,
/* 0x52 - Insert	*/	KEYBOARD_CODE_NUMPAD_INSERT,	'0',		KEYBOARD_CODE_INSERT,
/* 0x53 - Delete	*/	KEYBOARD_CODE_NUMPAD_DOT,	'.',		KEYBOARD_CODE_DELETE,
/* 0x54 - Enter		*/	0,		0,		0,
/* 0x55 - ???		*/	0,		0,		0,
/* 0x56 - ???		*/	0,		0,		0,
/* 0x57 - F11		*/	KEYBOARD_CODE_F11,		KEYBOARD_CODE_F11,		0,	
/* 0x58 - F12		*/	KEYBOARD_CODE_F12,		KEYBOARD_CODE_F12,		0,	
/* 0x59 - ???		*/	0,		0,		0,	
/* 0x5A - ???		*/	0,		0,		0,	
/* 0x5B - ???		*/	0,		0,		KEYBOARD_CODE_LGUI,	
/* 0x5C - ???		*/	0,		0,		KEYBOARD_CODE_RGUI,	
/* 0x5D - ???		*/	0,		0,		KEYBOARD_CODE_APPS,	
/* 0x5E - ???		*/	0,		0,		0,	
/* 0x5F - ???		*/	0,		0,		0,
/* 0x60 - ???		*/	0,		0,		0,
/* 0x61 - ???		*/	0,		0,		0,	
/* 0x62 - ???		*/	0,		0,		0,	
/* 0x63 - ???		*/	0,		0,		0,	
/* 0x64 - ???		*/	0,		0,		0,	
/* 0x65 - ???		*/	0,		0,		0,	
/* 0x66 - ???		*/	0,		0,		0,	
/* 0x67 - ???		*/	0,		0,		0,	
/* 0x68 - ???		*/	0,		0,		0,	
/* 0x69 - ???		*/	0,		0,		0,	
/* 0x6A - ???		*/	0,		0,		0,	
/* 0x6B - ???		*/	0,		0,		0,	
/* 0x6C - ???		*/	0,		0,		0,	
/* 0x6D - ???		*/	0,		0,		0,	
/* 0x6E - ???		*/	0,		0,		0,	
/* 0x6F - ???		*/	0,		0,		0,	
/* 0x70 - ???		*/	0,		0,		0,	
/* 0x71 - ???		*/	0,		0,		0,	
/* 0x72 - ???		*/	0,		0,		0,	
/* 0x73 - ???		*/	0,		0,		0,	
/* 0x74 - ???		*/	0,		0,		0,	
/* 0x75 - ???		*/	0,		0,		0,	
/* 0x76 - ???		*/	0,		0,		0,	
/* 0x77 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x7A - ???		*/	0,		0,		0,	
/* 0x7B - ???		*/	0,		0,		0,	
/* 0x7C - ???		*/	0,		0,		0,	
/* 0x7D - ???		*/	0,		0,		0,	
/* 0x7E - ???		*/	0,		0,		0,
/* 0x7F - ???		*/	0,		0,		0
};

void init_keyboard()
{
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);

    lshift = rshift = lalt = ralt = lctrl = rctrl = false;

    keyboard_in.count = 0;
    keyboard_in.head = keyboard_in.tail = keyboard_in.buffer;
}

void keyboard_handler(int irq)
{
    u8 scan_code = in_byte(KEYBOARD_IN_PORT);
    
    if (keyboard_in.count < KEYBOARD_IN_BUFFERSIZE)
    {
        *keyboard_in.head++ = scan_code;
        if (keyboard_in.head == keyboard_in.buffer + KEYBOARD_IN_BUFFERSIZE)
            keyboard_in.head = keyboard_in.buffer;
        ++keyboard_in.count;
    }
}

static u8 keyboard_readfrombuffer()
{
    while (keyboard_in.count <= 0)
        ;

    disable_int();
    u8 scan_code = *keyboard_in.tail++;
    if (keyboard_in.tail == keyboard_in.buffer + KEYBOARD_IN_BUFFERSIZE)
        keyboard_in.tail = keyboard_in.buffer;
    --keyboard_in.count;
    enable_int();

    return scan_code;
}

void keyboard_read()
{
	bool is_make;
	u32 key = 0;
	u32* keyrow;

	if (keyboard_in.count > 0)
    {
		code_with_e0 = 0;
		u8 scan_code = keyboard_readfrombuffer();

        // Parse scan code
		if (scan_code == 0xE1)
        {
			const u8 pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			bool is_pausebreak = 1;
			for (int i = 1; i < 6; ++i)
            {
				if (keyboard_readfrombuffer() != pausebrk_scode[i])
                {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak)
				key = KEYBOARD_CODE_PAUSEBREAK;
		}
		else if (scan_code == 0xE0)
        {
			scan_code = keyboard_readfrombuffer();

            // PrintScreen pressed
			if (scan_code == 0x2A)
            {
				if (keyboard_readfrombuffer() == 0xE0)
                {
					if (keyboard_readfrombuffer() == 0x37)
                    {
						key = KEYBOARD_CODE_PRINTSCREEN;
						is_make = true;
					}
				}
			}
			// PrintScreen released
			if (scan_code == 0xB7)
            {
				if (keyboard_readfrombuffer() == 0xE0)
                {
					if (keyboard_readfrombuffer() == 0xAA) {
						key = KEYBOARD_CODE_PRINTSCREEN;
						is_make = false;
					}
				}
			}

			// not PrintScreen
			if (key == 0) 
				code_with_e0 = true;
		}
		if ((key != KEYBOARD_CODE_PAUSEBREAK) && (key != KEYBOARD_CODE_PRINTSCREEN))
        {
			is_make = !(scan_code & KEYBOARD_FLAG_BREAK);

			keyrow = &keymap[(scan_code & 0x7F) * KEYBOARD_MAP_COLS];
			
			column = 0;
			if (lshift || rshift)
				column = 1;
			if (code_with_e0)
            {
				column = 2; 
				code_with_e0 = 0;
			}
			
			key = keyrow[column];
			
			switch(key)
            {
			case KEYBOARD_CODE_LSHIFT:
				lshift = is_make;
				break;
			case KEYBOARD_CODE_RSHIFT:
				rshift = is_make;
				break;
			case KEYBOARD_CODE_LCTRL:
				lctrl = is_make;
				break;
			case KEYBOARD_CODE_RCTRL:
				rctrl = is_make;
				break;
			case KEYBOARD_CODE_LALT:
				lalt = is_make;
				break;
			case KEYBOARD_CODE_RALT:
				ralt = is_make;
				break;
			default:
				break;
			}

			if (is_make)
            {
				key |= lshift ? KEYBOARD_FLAG_LSHIFT : 0;
				key |= rshift ? KEYBOARD_FLAG_RSHIFT : 0;
				key |= lctrl ? KEYBOARD_FLAG_LCTRL : 0;
				key |= rctrl ? KEYBOARD_FLAG_RCTRL : 0;
				key |= lalt ? KEYBOARD_FLAG_LALT : 0;
				key |= ralt ? KEYBOARD_FLAG_RALT : 0;
				in_process(key);
			}
		}
	}
}