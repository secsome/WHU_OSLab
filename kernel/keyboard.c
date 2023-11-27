#include <kernel/keyboard.h>
#include <kernel/protect.h>
#include <lib/display.h>
#include <lib/asm.h>

static keyboard_t keyboard_in;

void init_keyboard()
{
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);

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

void keyboard_read()
{
    if (keyboard_in.count > 0)
    {
        disable_int();

        u8 scan_code = *keyboard_in.tail++;
        if (keyboard_in.tail == keyboard_in.buffer + KEYBOARD_IN_BUFFERSIZE)
            keyboard_in.tail = keyboard_in.buffer;
        --keyboard_in.count;

        enable_int();

        disp_str("0x");
        disp_int(scan_code);
        disp_str(" ");
    }
}