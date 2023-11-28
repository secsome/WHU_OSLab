BITS 32

[SECTION .text]
; int lib_get_ticks()
global lib_get_ticks
lib_get_ticks:
    mov eax, 0
    int 0x90
    ret

; u32 lib_write(const char* buffer, u32 length)
global lib_write
lib_write:
    mov eax, 1
    int 0x90
    ret