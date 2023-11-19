#include "lib/display.h"
#include "lib/string.h"
#include "lib/asm.h"

int disp_pos;

PUBLIC void disp_reset()
{
	disp_pos = 0;
}

PUBLIC void disp_setpos(int pos)
{
	disp_pos = pos;
}

PUBLIC void disp_clear()
{
	for (int i = 0; i < 2 * 80 * 25; i += 4)
		write_gs_dword(i, 0);
}

PUBLIC void disp_clearlines(int line_count)
{
	for (int i = 0; i < 2 * 80 * line_count; i += 4)
		write_gs_dword(i, 0);
}

void disp_str(const char* info)
{
	union
	{
		struct
		{
			char ch;
			u8 color;
		};
		u16 word;
	} value;
	value.color = 0xF;
	for (;;)
	{
		value.ch = *info++;
		if (value.ch == NULL)
			break;
		if (value.ch == '\n')
			disp_pos = 160 * (disp_pos / 160 + 1);
		else
		{
			write_gs_word(disp_pos, value.word);
			disp_pos += 2;
		}
	}
}

void disp_color_str(const char* info, int color)
{
	union
	{
		struct
		{
			char ch;
			u8 color;
		};
		u16 word;
	} value;
	value.color = color;
	for (;;)
	{
		value.ch = *info++;
		if (value.ch == NULL)
			break;
		if (value.ch == '\n')
			disp_pos = 160 * (disp_pos / 160 + 1);
		else
		{
			write_gs_word(disp_pos, value.word);
			disp_pos += 2;
		}
	}
}

PUBLIC char* itoa(unsigned long value, char *buflim, unsigned int base, bool upper_case)
{
    static const char _itoa_upper_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char _itoa_lower_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const char *digits = (upper_case ? _itoa_upper_digits : _itoa_lower_digits);

	if (base > 36 || base < 2)
		return buflim;

    do
        *--buflim = digits[value % base];
    while ((value /= base) != 0);

    return buflim;
}


PUBLIC void disp_int(int input)
{
	char buffer[0x100] = { 0 };
	char* p = itoa(input, buffer + sizeof(buffer) - 1, 16, false);
	disp_str(p);
}