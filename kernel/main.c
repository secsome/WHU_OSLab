#include <lib/display.h>

PUBLIC void delay(int time)
{
    for (int t = 0; t < time; ++t)
    {
        for (int i = 0; i < 100000; ++i)
        {
            asm("xchg bx, bx");
        }
    }
}

PUBLIC int kernel_main()
{
    disp_clear();
    disp_reset();
    disp_str("----- kernel_main begins-----\n");

    while (true)
    {
        
    }
}

void TestA()
{
    int i = 0;
    while (true)
    {
        disp_str("A");
        delay(1);
    }
}