#include <kernel/tty.h>
#include <kernel/keyboard.h>

void task_tty()
{
    while (true)
    {
        keyboard_read();
    }
}