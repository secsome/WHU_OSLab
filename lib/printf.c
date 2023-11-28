#include <lib/syscall.h>
#include <lib/printf.h>
#include <lib/display.h>
#include <lib/string.h>
#include <lib/ctype.h>
#include <lib/stdlib.h>
#include <lib/wchar.h>

int printf(const char *fmt, ...)
{
	int i;
	char buf[256] = {0};

	va_list arg = (va_list)((char*)(&fmt) + 4);
	i = vsprintf(buf, fmt, arg);
       
	lib_write(buf, i);

	return i;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;
	char	tmp[256] = {0};
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
		case 'x':
            char* ptr = itoa(*((int*)p_next_arg), tmp + sizeof(tmp) - 1, 16, false);
			strcpy(p, ptr);
			p_next_arg += 4;
			p += strlen(ptr);
			break;
		case 's':
			break;
		default:
			break;
		}
	}

	return (p - buf);
}