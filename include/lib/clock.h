#pragma once

#include <sys/const.h>
#include <sys/type.h>

HEADER_CPP_BEGIN

u32 get_ticks();
void usleep(int millisecond);
void sleep(int second);

HEADER_CPP_END