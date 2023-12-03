#pragma once

#include <sys/const.h>

HEADER_CPP_BEGIN

#ifndef NULL
#define NULL (0)
#endif // NULL

long strtol(const char* nptr, char** endptr, int base);
int atoi(const char* nptr);

HEADER_CPP_END