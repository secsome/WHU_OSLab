#pragma once

#include <kernel/proto.h>

void disp_reset();
void disp_setpos(int pos);
void disp_clear();
void disp_clearlines(int line_count);
void disp_int(int value);
void	disp_str(const char* info);
void	disp_color_str(const char* info, int color);