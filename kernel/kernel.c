#include "kernel/const.h"
#include "lib/display.h"

__attribute__((section(".bss"))) char StackSpace[0x800];

// kernel entrypoint
void NAKED _start()
{
    
}