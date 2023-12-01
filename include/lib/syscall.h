#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/proc.h>

HEADER_CPP_BEGIN

FASTCALL u32 lib_get_ticks();
FASTCALL u32 lib_write(const char* buffer, u32 length);

HEADER_CPP_END