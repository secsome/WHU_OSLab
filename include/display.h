#pragma once

#include "proto.h"

PUBLIC void disp_reset();
PUBLIC void disp_setpos(int pos);
PUBLIC void disp_clear();
PUBLIC void disp_clearlines(int line_count);
PUBLIC void disp_int(int value);
PUBLIC void	disp_str(const char* info);
PUBLIC void	disp_color_str(const char* info, int color);