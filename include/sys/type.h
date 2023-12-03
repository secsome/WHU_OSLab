#pragma once

#include <sys/const.h>

HEADER_CPP_BEGIN

typedef unsigned long long  u64;
typedef	unsigned int        u32;
typedef	unsigned short      u16;
typedef	unsigned char       u8;

typedef void (*farproc_t)();

#ifndef TYPE_SIZE_T_DEFINED
    #define TYPE_SIZE_T_DEFINED
    typedef u32 size_t;
#endif // TYPE_SIZE_T_DEFINED

#ifndef TYPE_PTRDIFF_DEFINED
    #define TYPE_PTRDIFF_DEFINED
    typedef u32 ptrdiff_t;
#endif // TYPE_PTRDIFF_DEFINED

#ifndef __cplusplus
    #ifndef TYPE_BOOL_DEFINED
        #define TYPE_BOOL_DEFINED
        typedef u8  bool;

        #ifndef false
            #define false ((bool)0)
        #endif // false
        #ifndef true
            #define true ((bool)1)
        #endif // true
    #endif // TYPE_BOOL_DEFINED
#endif // __cplusplus

HEADER_CPP_END