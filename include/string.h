#pragma once

#include "const.h"
#include "type.h"

PUBLIC void* memcpy(void* p_dst, const void* p_src, size_t size);

PUBLIC void* memset(void* s, int c, size_t n);

// STRING(3)

// Copy a string from src to dest, returning a pointer to the end of the resulting string at dest.
PUBLIC char* stpcpy(char *dest, const char *src);

// Append the string src to the string dest, returning a pointer dest.
PUBLIC char* strcat(char *dest, const char *src);

// Return a pointer to the first occurrence of the character c in the string s.
PUBLIC char* strchr(const char *s, int c);

// Compare the strings s1 with s2.
PUBLIC int strcmp(const char *s1, const char *s2);

// Compare the strings s1 with s2 using the current locale.
PUBLIC int strcoll(const char *s1, const char *s2);

// Copy the string src to dest, returning a pointer to the start of dest.
PUBLIC char *strcpy(char *dest, const char *src);

// Calculate the length of the initial segment of the string s which does not contain any of bytes in the string reject,
PUBLIC size_t strcspn(const char *s, const char *reject);

// Return a duplicate of the string s in memory allocated using malloc(3).
PUBLIC char *strdup(const char *s);

// Randomly swap the characters in string.
PUBLIC char *strfry(char *string);

// Return the length of the string s.
PUBLIC size_t strlen(const char *s);

// Append at most n bytes from the string src to the string dest, returning a pointer to dest.
PUBLIC char *strncat(char *dest, const char *src, size_t n);

// Compare at most n bytes of the strings s1 and s2.
PUBLIC int strncmp(const char *s1, const char *s2, size_t n);

// Copy at most n bytes from string src to dest, returning a pointer to the start of dest.
PUBLIC char *strncpy(char *dest, const char *src, size_t n);

// Return a pointer to the first occurrence in the string s of one of the bytes in the string accept.
PUBLIC char *strpbrk(const char *s, const char *accept);

// Return a pointer to the last occurrence of the character c in the string s.
PUBLIC char *strrchr(const char *s, int c);

// Extract the initial token in stringp that is delimited by one of the bytes in delim.
PUBLIC char *strsep(char **stringp, const char *delim);

// Calculate the length of the starting segment in the string s that consists entirely of bytes in accept.
PUBLIC size_t strspn(const char *s, const char *accept);

// Find the first occurrence of the substring needle in the string haystack, returning a pointer to the found substring.
PUBLIC char *strstr(const char *haystack, const char *needle);

// Extract tokens from the string s that are delimited by one of the bytes in delim.
PUBLIC char *strtok(char *s, const char *delim);

// Transforms src to the current locale and copies the first n bytes to dest.
PUBLIC size_t strxfrm(char *dest, const char *src, size_t n);