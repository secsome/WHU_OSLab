#include <lib/puts.h>
#include <lib/syscall.h>
#include <lib/string.h>

int putchar(int c)
{
    return (int)lib_write((const char*)&c, 1);
}

int puts(const char* s)
{
    const int result = (int)lib_write(s, strlen(s));
    putchar('\n');
    return result;
}