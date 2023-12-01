#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

FASTCALL void out_byte(u16 port, u8 value);
FASTCALL u8 in_byte(u16 port);
FASTCALL void write_gs_byte(u32 pos, u8 value);
FASTCALL void write_gs_word(u32 pos, u16 value);
FASTCALL void write_gs_dword(u32 pos, u32 value);
FASTCALL void load_gdt(u32 address);
FASTCALL void save_gdt(u32 address);
FASTCALL void load_idt(u32 address);
FASTCALL void save_idt(u32 address);
FASTCALL void disable_int();
FASTCALL void enable_int();
FASTCALL bool init_fpu();

HEADER_CPP_END