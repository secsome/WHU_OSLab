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

