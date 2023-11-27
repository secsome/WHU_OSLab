#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <lib/display.h>

void task_tty()
{
    while (true)
    {
        keyboard_read();
    }
}

void in_process(u32 key)
{
    char output[2] = { 0 };
    if (!(key & KEYBOARD_FLAG_EXT))
    {
        output[0] = key & 0xff;
        disp_str(output);
    }
}