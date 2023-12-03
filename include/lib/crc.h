#pragma once

#include <kernel/const.h>
#include <lib/stdint.h>

HEADER_CPP_BEGIN

uint32_t crc32_calculate(const void* data, uint32_t length, uint32_t crc);

HEADER_CPP_END