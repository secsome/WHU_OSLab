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

char* strcpy(char* dest, const char* src)
{
    char* pdst = (char*)dest;
    char* psrc = (char*)src;

    while (*psrc)
        *(pdst++) = *(psrc++);
    *pdst = 0;
    
    return dest;
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