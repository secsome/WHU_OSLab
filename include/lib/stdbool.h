#pragma once

#include <kernel/const.h>

HEADER_CPP_BEGIN

#ifndef __cplusplus
    #ifndef TYPE_BOOL_DEFINED
    #define TYPE_BOOL_DEFINED
    typedef unsigned char bool;

    #ifndef false
        #define false ((bool)0)
    #endif
    #ifndef true
        #define true ((bool)1)
    #endif
    #endif // TYPE_BOOL_DEFINED
#endif // __cplusplus

HEADER_CPP_END