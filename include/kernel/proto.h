#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

PUBLIC void	init_prot();
PUBLIC void	init_8259A();
PUBLIC char* itoa(unsigned long value, char *buflim, unsigned int base, bool upper_case);