#pragma once

#include <lib/stdint.h>
#include <lib/va_list.h>

int printf(const char* format, ...);
int vprintf(const char* format, va_list ap);

int sprintf(char* str, const char* format, ...);
int vsprintf(char* str, const char* format, va_list ap);

int snprintf(char* str, size_t size, const char* format, ...);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);