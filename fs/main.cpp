#include <fs/core.h>
#include <fs/device.h>

#include <sys/sendrecv.h>
#include <lib/syscall.h>
#include <lib/assert.h>

void task_fs()
{
    lib_writex("Task FS begins.\n");

    // open the device: hard disk
    message_t driver_msg;
    driver_msg.type = SR_MSGTYPE_DEVOPEN;
    driver_msg.m_device.device = MINOR_DEV(FS_ROOT_DEVICE);
    const auto device = device_driver_map[MAJOR_DEV(FS_ROOT_DEVICE)];
    assert(device != FS_INVALID_DRIVER);

    sendrecv(SR_MODE_BOTH, device, &driver_msg);

    spin("FS");
};
