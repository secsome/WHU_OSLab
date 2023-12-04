#include <lib/clock.h>
#include <sys/clock.h>
#include <sys/systask.h>
#include <sys/sendrecv.h>
#include <lib/syscall.h>

u32 get_ticks()
{
    message_t msg = { 0 };
    msg.type = SR_MSGTYPE_GET_TICKS;
    sendrecv(SR_MODE_BOTH, TASK_SYS, &msg);
    return msg.m_int32;
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