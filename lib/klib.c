#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "global.h"

PUBLIC __attribute__((naked)) void out_byte(u16 port, u8 value)
{
	asm volatile
	(
		"mov dx, [esp + 4]\n"
		"mov al, [esp + 8]\n"
		"out dx, al\n"
		"nop\n"
		"nop\n"
		"ret"
	);
}

PUBLIC __attribute__((naked)) u8 in_byte(u16 port)
{
	asm volatile
	(
		"mov dx, [esp + 4]\n"
		"xor eax, eax\n"
		"in	al, dx\n"
		"nop\n"
		"nop\n"
		"ret"
	);
}

PUBLIC inline void write_gs_byte(u32 pos, u8 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}

PUBLIC inline void write_gs_word(u32 pos, u16 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}

PUBLIC inline void write_gs_dword(u32 pos, u32 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}
