#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

void	init_prot();
void	init_8259A();
char* itoa(unsigned long value, char *buflim, unsigned int base, bool upper_case);