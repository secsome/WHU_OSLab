#include <sys/systask.h>
#include <sys/sendrecv.h>
#include <sys/clock.h>
#include <lib/assert.h>

void task_sys()
{
    message_t msg;
    while (true)
    {
        sendrecv(SR_MODE_RECEIVE, SR_TARGET_ANY, &msg);

        switch (msg.type)
        {
        case SR_MSGTYPE_GET_TICKS:
            msg.m_int32 = sys_tick_count;
            sendrecv(SR_MODE_SEND, msg.source, &msg);
            break;
        default:
            panic("unknown msg type");
            break;
        }
    }
}

