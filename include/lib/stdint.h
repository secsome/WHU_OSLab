#pragma once

#include <kernel/const.h>

HEADER_CPP_BEGIN

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifndef TYPE_SIZE_T_DEFINED
#define TYPE_SIZE_T_DEFINED
typedef uint32_t size_t;
#endif // TYPE_SIZE_T_DEFINED

HEADER_CPP_END