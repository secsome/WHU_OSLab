#pragma once

#include <kernel/const.h>

HEADER_CPP_BEGIN

// Helpers
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#ifdef __cplusplus
    #define _ADDRESSOF(v) (&const_cast<char&>(reinterpret_cast<const volatile char&>(v)))
#else
    #define _ADDRESSOF(v) (&(v))
#endif // __cplusplus

#ifndef _UINTPTR_T_DEFINED
    #define _UINTPTR_T_DEFINED
    typedef unsigned int uintptr_t;
#endif // _UINTPTR_T_DEFINED

typedef char* va_list;

#ifdef __cplusplus
    extern "C++"
    {
        template <typename _Ty>
        struct __vcrt_va_list_is_reference
        {
            enum : bool { __the_value = false };
        };

        template <typename _Ty>
        struct __vcrt_va_list_is_reference<_Ty&>
        {
            enum : bool { __the_value = true };
        };

        template <typename _Ty>
        struct __vcrt_va_list_is_reference<_Ty&&>
        {
            enum : bool { __the_value = true };
        };

        template <typename _Ty>
        struct __vcrt_assert_va_start_is_not_reference
        {
            static_assert(!__vcrt_va_list_is_reference<_Ty>::__the_value,
                "va_start argument must not have reference type and must not be parenthesized");
        };
    } // extern "C++"

    #define va_start(ap, x) ((void)(__vcrt_assert_va_start_is_not_reference<decltype(x)>(), ((void)(ap = (va_list)_ADDRESSOF(x) + _INTSIZEOF(x)))))
#else 
    #define va_start(ap, v) ((void)(ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v)))
#endif

#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) ((void)(ap = (va_list)0))
#define va_copy(destination, source) ((destination) = (source))

HEADER_CPP_END