BITS 32

extern syscall_param0
extern syscall_param1
extern syscall_param2

[SECTION .text]
; u32 lib_sendrecv(u32 mode, u32 target, const message_t* msg)
global lib_sendrecv
lib_sendrecv:
    mov [syscall_param0], ecx
    mov [syscall_param1], edx
    mov eax, [esp + 4]
    mov [syscall_param2], eax
    mov eax, 0
    int 0x90
    ret 4

; u32 lib_writex(const char* s)
global lib_writex
lib_writex:
    mov [syscall_param0], ecx
    mov [syscall_param1], edx
    mov eax, 1
    int 0x90
    ret