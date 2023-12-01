#pragma once

#include <kernel/const.h>

HEADER_CPP_BEGIN

// Helpers
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define _ADDRESSOF(v) (&(v))

typedef char* va_list;

#define va_start(ap, v) ((void)(ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v)))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) ((void)(ap = (va_list)0))
#define va_copy(destination, source) ((destination) = (source))

HEADER_CPP_END