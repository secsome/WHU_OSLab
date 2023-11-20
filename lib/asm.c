#include <lib/asm.h>

NAKED void out_byte(u16 port, u8 value)
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

NAKED u8 in_byte(u16 port)
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

inline void write_gs_byte(u32 pos, u8 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}

inline void write_gs_word(u32 pos, u16 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}

inline void write_gs_dword(u32 pos, u32 value)
{
	asm
	(
		"mov gs:[%0], %1" : "+r"(pos) : "ri"(value)
	);
}

void load_gdt(u32 address)
{
	asm
	(
		"lgdt [%0]" : "+r"(address)
	);
}

void save_gdt(u32 address)
{
	asm
	(
		"sgdt [%0]" : "=r"(address)
	);
}

void load_idt(u32 address)
{
	asm
	(
		"lidt [%0]" : "+r"(address)
	);
}

void save_idt(u32 address)
{
	asm
	(
		"sidt [%0]" : "=r"(address)
	);
}