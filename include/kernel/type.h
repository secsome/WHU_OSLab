#pragma once

typedef	unsigned int    u32;
typedef	unsigned short  u16;
typedef	unsigned char   u8;

typedef void (*farproc_t)();

#ifndef TYPE_SIZE_T_DEFINED
#define TYPE_SIZE_T_DEFINED
typedef u32 size_t;
#endif // TYPE_SIZE_T_DEFINED

#ifndef TYPE_BOOL_DEFINED
#define TYPE_BOOL_DEFINED
typedef u8  bool;

#ifndef false
    #define false ((bool)0)
#endif
#ifndef true
    #define true ((bool)1)
#endif
#endif // TYPE_BOOL_DEFINED