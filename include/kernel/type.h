#pragma once

typedef	unsigned int    u32;
typedef	unsigned short  u16;
typedef	unsigned char   u8;

typedef void (*farproc_t)();

typedef u32 size_t;
typedef u8  bool;

#ifndef false
    #define false ((bool)0)
#endif
#ifndef true
    #define true ((bool)1)
#endif