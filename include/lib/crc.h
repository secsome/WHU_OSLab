#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

u32 crc32_calculate(const void* data, u32 length, u32 crc);

HEADER_CPP_END