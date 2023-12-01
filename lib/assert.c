#include <lib/assert.h>
#include <lib/printf.h>
#include <lib/stdbool.h>
#include <lib/asm.h>
#include <lib/syscall.h>
#include <lib/va_list.h>
#include <lib/string.h>

void spin(const char* name)
{
    char buffer[1024] = { 0 };
    strcpy(buffer, "\nspinning in ");
    strcat(buffer, name);
    strcat(buffer, " ...\n");
    lib_writex(buffer);
    while (true)
        ;
}

void assertion_failure(const char* expression, const char* file, const char* basefile, int line)
{
    char buffer[1024] = { 0 };
    snprintf(buffer, 1024, "\003assert(%s) failed: file: %s, base file: %s, line: %d\n", expression, file, basefile, line);
    lib_writex(buffer);
    spin("assertion_failure()");
    // should never arrive here
    ud2();
}

void panic(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024] = { 0 };
    vsnprintf(buffer, 1024, "\002!!panic!!: %s", args);
    va_end(args);
    lib_writex(buffer);
    // should never arrive here
    ud2();
}