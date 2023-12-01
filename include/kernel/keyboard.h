#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

enum
{
    KEYBOARD_PORT_DATA = 0x60,
    KEYBOARD_PORT_CMD = 0x64,
    KEYBOARD_LED = 0xED,
    KEYBOARD_IN_BUFFERSIZE = 32, // Size of keyboard input buffer
    KEYBOARD_MAP_COLS = 3, // Number of columns in keymap
    KEYBOARD_NUM_SCAN_CODES = 0x80, // Number of scan codes (rows in keymap)
    KEYBOARD_FLAG_BREAK = 0x80, // Break Code
    KEYBOARD_FLAG_EXT = 0x100, // Normal function keys
    KEYBOARD_FLAG_LSHIFT = 0x200,
    KEYBOARD_FLAG_RSHIFT = 0x400,
    KEYBOARD_FLAG_LCTRL = 0x800,
    KEYBOARD_FLAG_RCTRL = 0x1000,
    KEYBOARD_FLAG_LALT = 0x2000,
    KEYBOARD_FLAG_RALT = 0x4000,
    KEYBOARD_FLAG_NUMPAD = 0x8000, // Keys in num pad
    KEYBOARD_MASK_RAW = 0x01FF,

    KEYBOARD_CODE_ESC = KEYBOARD_FLAG_EXT + 0x01,
    KEYBOARD_CODE_TAB,
    KEYBOARD_CODE_ENTER,
    KEYBOARD_CODE_BACKSPACE,
    KEYBOARD_CODE_LGUI,
    KEYBOARD_CODE_RGUI,
    KEYBOARD_CODE_APPS,
    KEYBOARD_CODE_LSHIFT,
    KEYBOARD_CODE_RSHIFT,
    KEYBOARD_CODE_LCTRL,
    KEYBOARD_CODE_RCTRL,
    KEYBOARD_CODE_LALT,
    KEYBOARD_CODE_RALT,
    KEYBOARD_CODE_CAPSLOCK,
    KEYBOARD_CODE_NUMLOCK,
    KEYBOARD_CODE_SCROLLLOCK,
    KEYBOARD_CODE_F1,
    KEYBOARD_CODE_F2,
    KEYBOARD_CODE_F3,
    KEYBOARD_CODE_F4,
    KEYBOARD_CODE_F5,
    KEYBOARD_CODE_F6,
    KEYBOARD_CODE_F7,
    KEYBOARD_CODE_F8,
    KEYBOARD_CODE_F9,
    KEYBOARD_CODE_F10,
    KEYBOARD_CODE_F11,
    KEYBOARD_CODE_F12,
    KEYBOARD_CODE_PRINTSCREEN,
    KEYBOARD_CODE_PAUSEBREAK,
    KEYBOARD_CODE_INSERT,
    KEYBOARD_CODE_DELETE,
    KEYBOARD_CODE_HOME,
    KEYBOARD_CODE_END,
    KEYBOARD_CODE_PAGEUP,
    KEYBOARD_CODE_PAGEDOWN,
    KEYBOARD_CODE_UP,
    KEYBOARD_CODE_DOWN,
    KEYBOARD_CODE_LEFT,
    KEYBOARD_CODE_RIGHT,
    KEYBOARD_CODE_POWER,
    KEYBOARD_CODE_SLEEP,
    KEYBOARD_CODE_WAKE,
    KEYBOARD_CODE_NUMPAD_SLASH,
    KEYBOARD_CODE_NUMPAD_STAR,
    KEYBOARD_CODE_NUMPAD_MINUS,
    KEYBOARD_CODE_NUMPAD_PLUS,
    KEYBOARD_CODE_NUMPAD_ENTER,
    KEYBOARD_CODE_NUMPAD_DOT,
    KEYBOARD_CODE_NUMPAD_0,
    KEYBOARD_CODE_NUMPAD_1,
    KEYBOARD_CODE_NUMPAD_2,
    KEYBOARD_CODE_NUMPAD_3,
    KEYBOARD_CODE_NUMPAD_4,
    KEYBOARD_CODE_NUMPAD_5,
    KEYBOARD_CODE_NUMPAD_6,
    KEYBOARD_CODE_NUMPAD_7,
    KEYBOARD_CODE_NUMPAD_8,
    KEYBOARD_CODE_NUMPAD_9,
    KEYBOARD_CODE_NUMPAD_UP = KEYBOARD_CODE_NUMPAD_8,
    KEYBOARD_CODE_NUMPAD_DOWN = KEYBOARD_CODE_NUMPAD_2,
    KEYBOARD_CODE_NUMPAD_LEFT = KEYBOARD_CODE_NUMPAD_4,
    KEYBOARD_CODE_NUMPAD_RIGHT = KEYBOARD_CODE_NUMPAD_6,
    KEYBOARD_CODE_NUMPAD_HOME = KEYBOARD_CODE_NUMPAD_7,
    KEYBOARD_CODE_NUMPAD_END = KEYBOARD_CODE_NUMPAD_1,
    KEYBOARD_CODE_NUMPAD_PAGEUP = KEYBOARD_CODE_NUMPAD_9,
    KEYBOARD_CODE_NUMPAD_PAGEDOWN = KEYBOARD_CODE_NUMPAD_3,
    KEYBOARD_CODE_NUMPAD_INSERT = KEYBOARD_CODE_NUMPAD_0,
    KEYBOARD_CODE_NUMPAD_MIDDLE = KEYBOARD_CODE_NUMPAD_5,
    KEYBOARD_CODE_NUMPAD_DELETE = KEYBOARD_CODE_NUMPAD_DOT,
    KEYBOARD_CODE_ACK = 0xFA,
};

// Keyboard structure, 1 per console.
typedef struct keyboard_t
{
    u8* head; // Next space position in buffer
    u8* tail; // Current byte need to be processed in the buffer
    int count; // How many bytes do we have in buffer now
    u8 buffer[KEYBOARD_IN_BUFFERSIZE];
} keyboard_t;

extern u32 keymap[KEYBOARD_NUM_SCAN_CODES * KEYBOARD_MAP_COLS];

struct tty_t;

void init_keyboard();
void keyboard_handler(int irq);
void keyboard_read(struct tty_t* tty);

HEADER_CPP_END