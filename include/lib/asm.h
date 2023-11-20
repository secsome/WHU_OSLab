#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

void	out_byte(u16 port, u8 value);
u8 in_byte(u16 port);
void write_gs_byte(u32 pos, u8 value);
void write_gs_word(u32 pos, u16 value);
void write_gs_dword(u32 pos, u32 value);
void load_gdt(u32 address);
void save_gdt(u32 address);
void load_idt(u32 address);
void save_idt(u32 address);
