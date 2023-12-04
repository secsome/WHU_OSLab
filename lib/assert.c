#include <lib/assert.h>
#include <lib/printf.h>
#include <lib/stdbool.h>
#include <lib/asm.h>
#include <lib/printf.h>
#include <lib/va_list.h>
#include <lib/string.h>

void spin(const char* name)
{
    printl("\nspinning in %s ...\n", name);
    while (true)
        ;
}

void assertion_failure(const char* expression, const char* file, const char* basefile, int line)
{
    printl("\003assert(%s) failed: file: %s, base file: %s, line: %d\n", expression, file, basefile, line);
    spin("assertion_failure()");
    // should never arrive here
    ud2();
}

void panic(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintl(fmt, args);
    va_end(args);
    // should never arrive here
    ud2();
}