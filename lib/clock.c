#include <kernel/clock.h>
#include <lib/syscall.h>
#include <lib/clock.h>

void usleep(int millisecond)
{
    int t = lib_get_ticks();
    while(((lib_get_ticks() - t) * 1000 / CLOCK_HZ) < millisecond) {}
}

void sleep(int second)
{
    usleep(second * 1000);
}