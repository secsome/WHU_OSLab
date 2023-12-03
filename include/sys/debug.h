#pragma once

#include <sys/const.h>

HEADER_CPP_BEGIN

struct process_t;
void debug_dump_process(const struct process_t* process);

struct message_t;
void debug_dump_message(const char* title, const struct message_t* message);

HEADER_CPP_END