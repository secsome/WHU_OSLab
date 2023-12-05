#include <lib/puts.h>
#include <lib/syscall.h>
#include <lib/string.h>
#include <fs/fd.h>

int putchar(int c)
{
    write(STDOUT_FILENO, &c, 1);
    return c;
}

int puts(const char* s)
{
    const int result = write(STDOUT_FILENO, s, strlen(s));
    putchar('\n');
    return result;
}