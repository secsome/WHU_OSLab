#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/proc.h>
#include <kernel/sendrecv.h>

HEADER_CPP_BEGIN

FASTCALL u32 lib_write(const char* buffer, u32 length);
FASTCALL u32 lib_sendrecv(u32 mode, u32 target, const message_t* msg);
FASTCALL u32 lib_writex(const char* s);

HEADER_CPP_END