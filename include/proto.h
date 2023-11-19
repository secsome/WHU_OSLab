#pragma once

#include "const.h"
#include "type.h"

PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void	init_prot();
PUBLIC void	init_8259A();

PUBLIC char* itoa(unsigned long value, char *buflim, unsigned int base, bool upper_case);

PUBLIC void write_gs_byte(u32 pos, u8 value);
PUBLIC void write_gs_word(u32 pos, u16 value);
PUBLIC void write_gs_dword(u32 pos, u32 value);