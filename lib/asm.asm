BITS 32

[SECTION .text]
; void out_byte(u16 port, u8 value)
global out_byte
out_byte:
    mov al, dl
    mov dx, cx
    out dx, al
    ret

; u8 in_byte(u16 port)
global in_byte
in_byte:
    mov dx, cx
    xor eax, eax
    in al, dx
    ret

; void write_gs_byte(u32 pos, u8 value)
global write_gs_byte
write_gs_byte:
    mov gs:[ecx], dl
    ret

; void write_gs_word(u32 pos, u16 value)
global write_gs_word
write_gs_word:
    mov gs:[ecx], dx
    ret

; void write_gs_dword(u32 pos, u32 value)
global write_gs_dword
write_gs_dword:
    mov gs:[ecx], edx
    ret

; void load_gdt(u32 address)
global load_gdt
load_gdt:
    lgdt [ecx]
    ret

; void save_gdt(u32 address)
global save_gdt
save_gdt:
    sgdt [ecx]
    ret

; void load_idt(u32 address)
global load_idt
load_idt:
    lidt [ecx]
    ret

; void save_idt(u32 address)
global save_idt
save_idt:
    sidt [ecx]
    ret

; void disable_int()
global disable_int
disable_int:
    cli
    ret

; void enable_int()
global enable_int
enable_int:
    sti
    ret
