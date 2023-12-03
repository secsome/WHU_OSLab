#pragma once

#include <sys/const.h>

HEADER_CPP_BEGIN

void spin(const char* name);
void assertion_failure(const char* expression, const char* file, const char* basefile, int line);
void panic(const char* fmt, ...);

#ifndef assert
    #define assert(exp) ((exp) ? (void)0 : assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__))
#endif // assert

HEADER_CPP_END