#include <fs/core.h>
#include <kernel/sendrecv.h>
#include <lib/syscall.h>
#include <lib/assert.h>

void task_fs()
{
    lib_writex("Task FS begins.\n");

    // open the device: hard disk
    message_t driver_msg;
    driver_msg.type = SR_MSGTYPE_DEVOPEN;
    sendrecv(SR_MODE_BOTH, TASK_HARDDISK, &driver_msg);

    spin("FS");
};
