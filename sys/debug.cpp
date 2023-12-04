#include <sys/type.h>
#include <sys/debug.h>
#include <sys/proc.h>
#include <sys/console.h>
#include <sys/sendrecv.h>

#include <lib/printf.h>
#include <lib/asm.h>
#include <lib/syscall.h>
#include <lib/display.h>
#include <lib/string.h>

void debug_dump_process(const struct process_t* process)
{
    char buffer[1024] = { 0 };
	const auto text_color = MAKE_COLOR(COLOR_GREEN, COLOR_RED);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

    snprintf(buffer, sizeof(buffer), "byte dump of proc_table[%d]:\n", static_cast<int>(process - proc_table));
    disp_color_str(buffer, text_color);
	for (size_t i = 0; i < sizeof(process_t); ++i)
    {
		snprintf(buffer, sizeof(buffer), "%02X ", ((unsigned char *)process)[i]);
		disp_color_str(buffer, text_color);
	}

    snprintf(buffer, sizeof(buffer),
        "\n\nANY: 0x%02X, NO_TASK: 0x%02X.\n\n"
        "ldt_selector: 0x%04X, ticks: 0x%08X, priority: 0x%08X, pid: 0x%08X, process_name: %s.\n"
        "process_flags: 0x%08X, recvfrom: 0x%08X, sendto: 0x%08X, tty_index: 0x%08X, has_int_msg: 0x%08X.\n",
        SR_TARGET_ANY, SR_TARGET_NONE,
        process->ldt_selector, process->ticks, process->priority, process->pid, process->process_name,
        process->process_flags, process->recvfrom, process->sendto, process->tty_index, process->has_int_msg);
    disp_color_str(buffer, text_color);
}

void debug_dump_message(const char* title, const struct message_t* message)
{
    constexpr bool packed = false;
	constexpr const char* space = packed ? "" : "\n        ";
	printl("{%s}<0x%08X>{%ssrc:%s(%d),%stype:%d,%s(\n",
		title, message, space, proc_table[message->source].process_name, message->source, space, message->type, space);
	for (size_t i = 0; i < sizeof(message_t) - 8; ++i)
		printl("%02X ", message->m_raw[i]);
	printl("\n)%s}%s", space, space);
}