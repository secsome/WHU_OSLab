#include <kernel/syscall.h>
#include <lib/syscall.h>

int NAKED lib_get_ticks()
{
    asm
    (
        "mov eax, 0\n"  // SYSCALL_GETTICKS
        "int 0x90\n"    // SYSCALL_NUMINT
        "ret"
    );
}