#include <kernel/keyboard.h>
#include <kernel/protect.h>
#include <lib/display.h>
#include <lib/asm.h>

void init_keyboard()
{
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}

void keyboard_handler(int irq)
{
    u8 ch = in_byte(0x60);
    disp_str("0x");
    disp_int(ch);
    disp_str(" ");
}