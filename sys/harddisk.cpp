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
#include <lib/string.h>

#include <lib/algorithm>

#include <fs/device.h>

static void harddisk_init();
static void harddisk_open(int device);
static void harddisk_close(int device);
static void harddisk_readwrite(const message_t& msg);
static void harddisk_ioctl(const message_t& msg);
static void harddisk_command_out(harddisk_command_t* cmd);
static void harddisk_get_partition_table(int drive, int section_count, partition_entry_t* entry);
static void harddisk_partition(int device, int style);
static void harddisk_print_harddisk_info(const harddisk_info_t* info);
static bool harddisk_waitfor(int mask, int val, size_t timeout);
static void harddisk_interrupt_wait();
static void harddisk_identify(int drive);
static void harddisk_print_identify_info(u16* hdinfo);
static void harddisk_handler(int irq);

static u8 harddisk_status;
static u8 harddisk_buffer[HD_SECTOR_SIZE * 2];
static harddisk_info_t harddisk_info[1];

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
            harddisk_open(msg.m_device.device);
            break;
		case SR_MSGTYPE_DEVCLOSE:
			harddisk_close(msg.m_device.device);
			break;
		case SR_MSGTYPE_DEVREAD:
		case SR_MSGTYPE_DEVWRITE:
			harddisk_readwrite(msg);
			break;
		case SR_MSGTYPE_DEVIOCTL:
			harddisk_ioctl(msg);
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
    printl("Drive count:%d.\n", drive_count);
    assert(drive_count);

    put_irq_handler(AT_WINI_IRQ, harddisk_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);

	for (auto& info : harddisk_info)
		memset(&info, 0, sizeof(info));
	harddisk_info[0].open_count = 0;
}

static void harddisk_open(int device)
{
	const int drive = GET_DEV_DRIVER(device);
	assert(drive == 0); // We have only one drive

	harddisk_identify(drive);

	auto& info = harddisk_info[drive];
	if (info.open_count++ == 0)
	{
		harddisk_partition(drive * (HD_NUM_PART_PER_DRIVE + 1), HD_PARTITION_STYLE_PRIMARY);
		harddisk_print_harddisk_info(&info);
	}
}

static void harddisk_close(int device)
{
	const int drive = GET_DEV_DRIVER(device);
	assert(drive == 0);	// We have only one drive

	--harddisk_info[drive].open_count;
}

static void harddisk_readwrite(const message_t& msg)
{
	const int drive = GET_DEV_DRIVER(msg.m_device.device);
	const auto position = msg.m_device.position;
	assert((position >> HD_SECTOR_SIZE_SHIFT) < (1u << 31));
	
	// We only allow to R/W from a SECTOR boundary
	assert((position & 0x1FF) == 0);

	size_t sector_count = static_cast<size_t>(position >> HD_SECTOR_SIZE_SHIFT);
	const int logical_index = (msg.m_device.device - HD_MINOR_1A) % HD_NUM_SUB_PER_DRIVE;
	sector_count += msg.m_device.device < HD_MAX_PRIM ?
		harddisk_info[drive].primary[msg.m_device.device].base :
		harddisk_info[drive].logical[logical_index].base;
	
	harddisk_command_t cmd;
	cmd.features = 0;
	cmd.sector_count = (msg.m_device.count + HD_SECTOR_SIZE - 1) / HD_SECTOR_SIZE;
	cmd.lba_low = sector_count & 0xFF;
	cmd.lba_mid = (sector_count >> 8) & 0xFF;
	cmd.lba_high = (sector_count >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, drive, (sector_count >> 24) & 0xF);
	cmd.command = msg.type == SR_MSGTYPE_DEVREAD ? HD_ATA_READ : HD_ATA_WRITE;
	harddisk_command_out(&cmd);

	int bytes_left = msg.m_device.count;
	auto la = reinterpret_cast<char*>(va2la(msg.source, msg.m_device.buffer));

	while (bytes_left)
	{
		int bytes = std::min(HD_SECTOR_SIZE, bytes_left);
		if (msg.type == SR_MSGTYPE_DEVREAD)
		{
			harddisk_interrupt_wait();
			port_read(HD_REG_DATA, harddisk_buffer, HD_SECTOR_SIZE);
			memcpy(la, va2la(TASK_HARDDISK, harddisk_buffer), bytes);
		}
		else
		{
			if (!harddisk_waitfor(HD_STATUS_DRQ, HD_STATUS_DRQ, HD_TIMEOUT))
				panic("harddisk writing error.");

			port_write(HD_REG_DATA, la, bytes);
			harddisk_interrupt_wait();
		}
		bytes_left -= HD_SECTOR_SIZE;
		la += HD_SECTOR_SIZE;
	}
}

static void harddisk_ioctl(const message_t& msg)
{
	const int device = msg.m_device.device;
	const int drive = GET_DEV_DRIVER(device);

	const auto& info = harddisk_info[drive];

	if (msg.m_device.request == HD_IOCTL_GET_GEO) 
	{
		void* dst = va2la(msg.m_device.process_index, msg.m_device.buffer);
		void* src = va2la(TASK_HARDDISK,
			device < HD_MAX_PRIM ?
			&info.primary[device] :
			&info.logical[(device - HD_MINOR_1A) % HD_NUM_SUB_PER_DRIVE]);
		memcpy(dst, src, sizeof(partition_info_t));
	}
	else
		assert(false);
}

static void harddisk_get_partition_table(int drive, int section_count, partition_entry_t* entry)
{
	harddisk_command_t cmd;
	cmd.features = 0;
	cmd.sector_count = 1;
	cmd.lba_low = section_count & 0xFF;
	cmd.lba_mid = (section_count >> 8) & 0xFF;
	cmd.lba_high = (section_count >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, drive, (section_count >> 24) & 0xF);
	cmd.command = HD_ATA_READ;
	harddisk_command_out(&cmd);
	harddisk_interrupt_wait();

	port_read(HD_REG_DATA, harddisk_buffer, HD_SECTOR_SIZE);
	memcpy(entry, harddisk_buffer + HD_PARTITION_TABLE_OFFSET, sizeof(partition_entry_t) * HD_NUM_PART_PER_DRIVE);
}

static void harddisk_partition(int device, int style)
{
	const int drive = GET_DEV_DRIVER(device);
	auto& info = harddisk_info[drive];
	partition_entry_t partition_table[HD_NUM_SUB_PER_DRIVE];

	if (style == HD_PARTITION_STYLE_PRIMARY)
	{
		harddisk_get_partition_table(drive, drive, partition_table);
		int primary_part_count = 0;
		for (int i = 0; i < HD_NUM_PART_PER_DRIVE; ++i)
		{
			if(partition_table[i].system_id == HD_PARTITION_TYPE_NONE)
				continue;
			
			++primary_part_count;
			const int device_index = i + 1;
			info.primary[device_index].base = partition_table[i].starting_sector32;
			info.primary[device_index].size = partition_table[i].number_of_sectors32;

			if (partition_table[i].system_id == HD_PARTITION_TYPE_EXTENDED)
				harddisk_partition(device + device_index, HD_PARTITION_STYLE_EXTENDED);
		}
		assert(primary_part_count != 0);
	}
	else if (style == HD_PARTITION_STYLE_EXTENDED)
	{
		const int primary_index = device % HD_NUM_PRIM_PER_DRIVE;
		const int start_sector = info.primary[primary_index].base;
		int base_sector = start_sector;
		int first_sub_index = (primary_index - 1) * HD_NUM_SUB_PER_PART; /* 0/16/32/48 */

		for (int i = 0; i < HD_NUM_SUB_PER_PART; ++i)
		{
			harddisk_get_partition_table(drive, base_sector, partition_table);

			const int device_index = first_sub_index + i;
			info.logical[device_index].base = base_sector + partition_table[0].starting_sector32;
			info.logical[device_index].size = partition_table[0].number_of_sectors32;

			base_sector = start_sector + partition_table[1].starting_sector32;

			// No more logical partitions in this extended partition
			if (partition_table[1].system_id == HD_PARTITION_TYPE_NONE)
				break;
		}
	}
	else
		assert(false);
}

static void harddisk_print_harddisk_info(const harddisk_info_t* info)
{
	for (int i = 0; i < HD_NUM_PART_PER_DRIVE + 1; ++i)
	{
		printl("%sPART_%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i == 0 ? " " : "     ",
			i,
			info->primary[i].base,
			info->primary[i].base,
			info->primary[i].size,
			info->primary[i].size);
	}
	
	for (int i = 0; i < HD_NUM_SUB_PER_DRIVE; ++i)
	{
		if (info->logical[i].size == 0)
			continue;
		printl("         "
			"%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i,
			info->logical[i].base,
			info->logical[i].base,
			info->logical[i].size,
			info->logical[i].size);
	}
}

static void harddisk_identify(int drive)
{
	harddisk_command_t cmd;
	cmd.device = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = HD_ATA_IDENTIFY;
	harddisk_command_out(&cmd);
	harddisk_interrupt_wait();
	port_read(HD_REG_DATA, harddisk_buffer, HD_SECTOR_SIZE);
	
	const auto hdinfo = reinterpret_cast<u16*>(harddisk_buffer);
	harddisk_print_identify_info(hdinfo);
	harddisk_info[drive].primary[0].base = 0;
	harddisk_info[drive].primary[0].size = ((int)hdinfo[61] << 16) + hdinfo[60];
}

static void harddisk_print_identify_info(u16* hdinfo)
{
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
		
        printl("%s: %s\n", info.desc, s);
	}

	const int capabilities = hdinfo[49];
    printl("LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

	const int cmd_set_supported = hdinfo[83];
    printl("LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

	const int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    printl("HD size: %dMB\n", sectors * 512 / 1000000);
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