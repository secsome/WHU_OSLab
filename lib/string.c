#include <lib/string.h>

void* memcpy(void* dest, const void* src, size_t n)
{
    u8* pdst = (u8*)dest;
    u8* psrc = (u8*)src;

    while (n--)
        *(pdst++) = *(psrc++);
    
    return dest;
}

void* memset(void* s, int c, size_t n)
{
    u8* ptr = (u8*)s;
    
    while (n--)
        *(ptr++) = (u8)c;
    
    return ptr;
}

char* stpcpy(char* dest, const char* src)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;

    while (*psrc)
        *(pdst++) = *(psrc++);
    *pdst = 0;
    
    return pdst;
}

char* strcat(char* dest, const char* src)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;
    
    while (*pdst++);

    while (*psrc)
        *(pdst++) = *(psrc++);
    *pdst = 0;

    return dest;
}

char* strchr(const char* s, int c)
{
    for (char ch = c; ; ++s) 
    {
		if (*s == ch)
			return (char*)s;
		if (*s == 0)
			return NULL;
	}
}

int strcmp(const char* s1, const char* s2)
{
    u8 u1, u2;

    for ( ; ; s1++, s2++) 
    {
	    u1 = (u8)*s1;
	    u2 = (u8)*s2;
	    if ((u1 == 0) || (u1 != u2))
	        break;
    }
    return u1 - u2;
}

char* strcpy(char* dest, const char* src)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;

    while (*psrc)
        *(pdst++) = *(psrc++);
    *pdst = 0;
    
    return dest;
}

size_t strcspn(const char* s, const char* reject)
{
    const char* p = s;
    const char* spanp;
	char c, sc;

	// Stop as soon as we find any character from s2.  Note that there
	// must be a NUL in s2; it suffices to stop when we find that, too.
    while (true)
    {
        c = *p++;
		spanp = reject;
		do 
        {
			if ((sc = *spanp++) == c)
				return p - 1 - s;
		} while (sc != 0);
    }
}

size_t strlen(const char* s)
{
    size_t length = 0;
    while (*s++)
        ++length;
    return length;
}

char* strncat(char* dest, const char* src, size_t n)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;
    
    while (*pdst++);

    while (*psrc && --n)
        *(pdst++) = *(psrc++);
    *pdst = 0;

    return dest;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    u8 u1, u2;

    for (size_t i = 0 ; i < n; s1++, s2++) 
    {
	    u1 = (u8)*s1;
	    u2 = (u8)*s2;
	    if ((u1 == 0) || (u1 != u2))
	        break;
    }
    return u1 - u2;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;

    while (*psrc && --n)
        *(pdst++) = *(psrc++);
    *pdst = 0;
    
    return dest;
}

char* strpbrk(const char* s, const char* accept)
{
    const char *scanp;
	int c, sc;

	while ((c = *s++) != 0) 
    {
		for (scanp = accept; (sc = *scanp++) != 0; )
		{
            if (sc == c)
				return (char *)(s - 1);
        }
	}
	return NULL;
}

char* strrchr(const char* s, int c)
{
	for (char* save = NULL; ; ++s) 
    {
		if (*s == c)
			save = (char *)s;
		if (!*s)
			return save;
	}
}

char* strsep(char* *stringp, const char* delim)
{
    char *s;

	if ((s = *stringp) == NULL)
		return NULL;
    
    char* tok = s;
	while (true)
    {
		int c = *s++;
        const char *spanp = delim;
		int sc;
        do 
        {
			if ((sc = *spanp++) == c) 
            {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return tok;
			}
		} while (sc != 0);
	}
}

size_t strspn(const char* s, const char* accept)
{
    const char* p = s;
    const char* spanp;
	char c, sc;

	// Skip any characters in s2, excluding the terminating \0.
cont:
	c = *p++;
	for (spanp = accept; (sc = *spanp++) != 0;)
	{
        if (sc == c)
			goto cont;
    }
	return p - 1 - s;
}

char* strstr(const char* haystack, const char* needle)
{
    // First scan quickly through the two strings looking for a single-character match.
    // When it's found, then compare the rest of the substring.
    char* b = (char*)needle;
    if (*b == 0)
	    return (char*)haystack;

    for ( ; *haystack != 0; haystack++) 
    {
	    if (*haystack != *b)
	        continue;
	    char* a = (char*)haystack;
	    while (1) 
        {
	        if (*b == 0)
		        return (char*)haystack;
	        if (*a++ != *b++)
		        break;
	    }
        b = (char*)needle;
    }
    return NULL;
}

char* strtok(char* s, const char* delim)
{
	register int c, sc;
	char *tok;
	static char *last;


	if (s == NULL && (s = last) == NULL)
		return NULL;

	
	// Skip (span) leading delimiters (s += strspn(s, delim), sort of).
cont:
	c = *s++;
	for (char* spanp = (char*)delim; (sc = *spanp++) != 0;) 
    {
		if (c == sc)
			goto cont;
	}

	if (c == 0) // no non-delimiter characters
    {
		last = NULL;
		return NULL;
	}
	tok = s - 1;

	// Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	// Note that delim must have one NUL; we stop if we see that, too.
	for (;;) 
    {
		c = *s++;
		char* spanp = (char*)delim;
		do 
        {
			if ((sc = *spanp++) == c) 
            {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				last = s;
				return tok;
			}
		} while (sc != 0);
	}
}