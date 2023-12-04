#include <fs/core.h>
#include <fs/device.h>

#include <sys/sendrecv.h>

#include <lib/assert.h>

void read_sector(int device, int sector, void* buffer)
{
    readwrite_sector(SR_MSGTYPE_DEVREAD, device, sector * HD_SECTOR_SIZE, HD_SECTOR_SIZE, TASK_FILESYSTEM, buffer);
}

void write_sector(int device, int sector, const void* buffer)
{
    readwrite_sector(SR_MSGTYPE_DEVWRITE, device, sector * HD_SECTOR_SIZE, HD_SECTOR_SIZE, TASK_FILESYSTEM, const_cast<void*>(buffer));
}

void readwrite_sector(int type, int device, u64 position, int count, int proccess_index, void* buffer)
{
    message_t msg;
    msg.type = type;
    msg.m_device.device = MINOR_DEV(device);
    msg.m_device.position = position;
    msg.m_device.buffer = buffer;
    msg.m_device.count = count;
    msg.m_device.process_index = proccess_index;

    const auto driver_index = device_driver_map[MAJOR_DEV(device)];
    assert(driver_index != FS_INVALID_DRIVER);

    sendrecv(SR_MODE_BOTH, driver_index, &msg);
}