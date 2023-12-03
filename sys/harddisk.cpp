#include <sys/harddisk.h>
#include <sys/sendrecv.h>
#include <sys/debug.h>
#include <sys/proc.h>
#include <sys/clock.h>

#include <lib/assert.h>
#include <lib/printf.h>
#include <lib/syscall.h>
#include <lib/asm.h>
#include <lib/clock.h>

static void harddisk_init();
static void harddisk_command_out(harddisk_command_t* cmd);
static bool harddisk_waitfor(int mask, int val, size_t timeout);
static void harddisk_interrupt_wait();
static void harddisk_identify(int drive);
static void harddisk_print_identify_info(u16* hdinfo);
static void harddisk_handler(int irq);

static u8 harddisk_status;
static u8 harddisk_buffer[HD_SECTOR_SIZE * 2];

void task_harddisk()
{
    harddisk_init();

    while (true)
    {
        message_t msg;
        sendrecv(SR_MODE_RECEIVE, SR_TARGET_ANY, &msg);

        switch (msg.type) 
        {
        case SR_MSGTYPE_DEVOPEN:
            harddisk_identify(0);
            break;
        default:
            debug_dump_message("Harddisk driver::unknown msg", &msg);
            spin("FS::main_loop (invalid msg.type)");
            break;
        }

        sendrecv(SR_MODE_SEND, msg.source, &msg);
    }

}

static void harddisk_init()
{
    // Get the number of drives from the BIOS data area
    const u8 drive_count = *(u8*)(0x475);
    char buffer[64] = { 0 };
    snprintf(buffer, sizeof(buffer), "Drive count:%d.\n", drive_count);
	lib_writex(buffer);
    assert(drive_count);

    put_irq_handler(AT_WINI_IRQ, harddisk_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
}

static void harddisk_identify(int drive)
{
	harddisk_command_t cmd;
	cmd.device  = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = HD_ATA_IDENTIFY;
	harddisk_command_out(&cmd);
	harddisk_interrupt_wait();
	port_read(HD_REG_DATA, harddisk_buffer, HD_SECTOR_SIZE);
	harddisk_print_identify_info((u16*)harddisk_buffer);
}

static void harddisk_print_identify_info(u16* hdinfo)
{
    char buffer[64] = { 0 };

	struct identify_info_ascii_t
	{
		int idx;
		int len;
		const char* desc;
	} iinfo[] = 
	{
		{10, 20, "HD SN"}, /* Serial number in ASCII */
		{27, 40, "HD Model"} /* Model number in ASCII */
	};

	for (const auto& info : iinfo)
	{
		char* p = (char*)&hdinfo[info.idx];
		int i;
		char s[64];
		for (i = 0; i < info.len / 2; ++i)
		{
			s[i * 2 + 1] = *p++;
			s[i * 2] = *p++;
		}
		s[i * 2] = 0;
        snprintf(buffer, sizeof(buffer), "%s: %s\n", info.desc, s);
		lib_writex(buffer);
	}

	const int capabilities = hdinfo[49];
    snprintf(buffer, sizeof(buffer), "LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");
    lib_writex(buffer);
    
	const int cmd_set_supported = hdinfo[83];
    snprintf(buffer, sizeof(buffer), "LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");
    lib_writex(buffer);
	
	const int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    snprintf(buffer, sizeof(buffer), "HD size: %dMB\n", sectors * 512 / 1000000);
    lib_writex(buffer);
}

static void harddisk_command_out(harddisk_command_t* cmd)
{
	// For all commands, the host must first check if BSY=1,
	// and should proceed no further unless and until BSY=0
	if (!harddisk_waitfor(HD_STATUS_BSY, 0, HD_TIMEOUT))
		panic("harddisk error.");

	// Activate the Interrupt Enable (nIEN) bit
	out_byte(HD_REG_DEV_CTRL, 0);

	// Load required parameters in the Command Block Registers
	out_byte(HD_REG_FEATURES, cmd->features);
	out_byte(HD_REG_NSECTOR, cmd->sector_count);
	out_byte(HD_REG_LBA_LOW, cmd->lba_low);
	out_byte(HD_REG_LBA_MID, cmd->lba_mid);
	out_byte(HD_REG_LBA_HIGH, cmd->lba_high);
	out_byte(HD_REG_DEVICE, cmd->device);

	// Write the command code to the Command Register
	out_byte(HD_REG_CMD, cmd->command);
}

static void harddisk_interrupt_wait()
{
    message_t msg;
    sendrecv(SR_MODE_RECEIVE, SR_TARGET_INTERRUPT, &msg);
}

static bool harddisk_waitfor(int mask, int val, size_t timeout)
{
	const auto t = get_ticks();

	while(((get_ticks() - t) * 1000 / CLOCK_HZ) < timeout)
	{
        if ((in_byte(HD_REG_STATUS) & mask) == val)
			return true;
    }

	return false;
}

static void harddisk_handler(int irq)
{
    harddisk_status = in_byte(HD_REG_STATUS);

	process_inform_interrupt(TASK_HARDDISK);
}