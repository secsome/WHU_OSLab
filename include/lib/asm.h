#pragma once

#include "kernel/const.h"
#include "kernel/type.h"

PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void write_gs_byte(u32 pos, u8 value);
PUBLIC void write_gs_word(u32 pos, u16 value);
PUBLIC void write_gs_dword(u32 pos, u32 value);
PUBLIC void load_gdt(u32 address);
PUBLIC void save_gdt(u32 address);
PUBLIC void load_idt(u32 address);
PUBLIC void save_idt(u32 address);
