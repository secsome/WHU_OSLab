#include <lib/clock.h>
#include <kernel/clock.h>
#include <kernel/systask.h>
#include <kernel/sendrecv.h>
#include <lib/syscall.h>

u32 get_ticks()
{
    message_t msg = { 0 };
    msg.type = SR_MSGTYPE_GET_TICKS;
    sendrecv(SR_MODE_BOTH, TASK_SYS, &msg);
    return msg.m3.m3i1;
}

void usleep(int millisecond)
{
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / CLOCK_HZ) < millisecond) {}
}

void sleep(int second)
{
    usleep(second * 1000);
}