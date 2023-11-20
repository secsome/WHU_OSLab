#pragma once

#include <kernel/type.h>

u32 crc32_calculate(const void* data, u32 length, u32 crc);