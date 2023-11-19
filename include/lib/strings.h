#pragma once

#include "kernel/const.h"
#include "kernel/type.h"

// STRING(3)
// Compare the strings s1 and s2 ignoring case.
PUBLIC int strcasecmp(const char* s1, const char* s2);

// Compare the first n bytes of the strings s1 and s2 ignoring case.
PUBLIC int strncasecmp(const char *s1, const char *s2, size_t n);

// Return a pointer to the first occurrence of the character c in the string s.
PUBLIC char *index(const char *s, int c);

// Return a pointer to the last occurrence of the character c in the string s.
PUBLIC char *rindex(const char *s, int c);
