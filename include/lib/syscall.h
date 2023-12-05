#pragma once

#include <sys/const.h>
#include <sys/type.h>
#include <sys/proc.h>
#include <sys/sendrecv.h>

HEADER_CPP_BEGIN

FASTCALL u32 lib_sendrecv(u32 mode, u32 target, const message_t* msg);
FASTCALL u32 lib_writex(const char* s);

HEADER_CPP_END