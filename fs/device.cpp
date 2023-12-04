#include <fs/device.h>
#include <sys/proc.h>

dev_driver_map_t device_driver_map[] = 
{
    FS_INVALID_DRIVER,  // FS_DEV_NONE
    FS_INVALID_DRIVER,  // FS_DEV_FLOPPY
    FS_INVALID_DRIVER,  // FS_DEV_CDROM
    TASK_HARDDISK,      // FS_DEV_HARDDISK
    TASK_TTY,           // FS_DEV_CHAR_TTY
    FS_INVALID_DRIVER,  // FS_DEV_SCSI
};

