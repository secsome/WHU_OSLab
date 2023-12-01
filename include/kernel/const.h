#pragma once

#define FORCEINLINE __attribute__((always_inline))

#ifndef NULL
    #define NULL (0)
#endif // NULL

#ifndef NAKED
    #define NAKED __attribute__((naked))
#endif // NAKED

#ifndef FASTCALL
    #define FASTCALL __attribute__((fastcall))
#endif // FASTCALL

#ifndef CFUNCTION
    #define CFUNCTION extern "C"
#endif // CFUNCTION

#ifndef HEADER_CPP_BEGIN
    #ifdef __cplusplus
        #define HEADER_CPP_BEGIN extern "C" {
    #else
        #define HEADER_CPP_BEGIN
    #endif 
#endif // HEADER_CPP_BEGIN

#ifndef HEADER_CPP_END
    #ifdef __cplusplus
        #define HEADER_CPP_END }
    #else
        #define HEADER_CPP_END
    #endif
#endif // HEADER_CPP_END

/* GDT 和 IDT 中描述符的个数 */
enum 
{
    GDT_SIZE = 128,
    IDT_SIZE = 256
};

/* 权限 */
enum
{
    PRIVILEGE_KRNL = 0,
    PRIVILEGE_TASK = 1,
    PRIVILEGE_USER = 3
};