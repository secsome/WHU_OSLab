#pragma once

#include <kernel/const.h>
#include <kernel/proto.h>

HEADER_CPP_BEGIN

void disp_reset();
void disp_setpos(int pos);
void disp_clear();
void disp_clearlines(int line_count);
void disp_int(int value);
void disp_color_int(int value, int color);
void disp_str(const char* info);
void disp_color_str(const char* info, int color);

HEADER_CPP_END