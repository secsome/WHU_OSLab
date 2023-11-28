#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/proc.h>

FASTCALL u32 lib_get_ticks();
FASTCALL u32 lib_write(const char* buffer, u32 length);