#pragma once

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