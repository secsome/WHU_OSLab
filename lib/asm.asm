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

; bool init_fpu()
fpu_test_word dw 0x55AA
global init_fpu
init_fpu:
    mov eax, cr0
    or eax, 00000000000000000000000000100010b
    mov cr0, eax
    fninit
    fnstsw [fpu_test_word]
    cmp word [fpu_test_word], 0
    xor eax, eax
    jne .no_fpu
    mov al, 1
.no_fpu:
    ret

; void halt()
global halt
halt:
    hlt
    ret

; void ud2()
global ud2
ud2:
    ud2
    ret

; void port_read(u16 port, void* buffer, u32 size)
global port_read
port_read:
    push edi
    mov edi, edx ; buffer
    mov edx, ecx ; port
    mov ecx, [esp + 8] ; size
    shr ecx, 1
    cld
    rep insw
    pop edi
    ret

; void port_write(u16 port, const void* buffer, u32 size)
global port_write
port_write:
    push esi
    mov esi, edx ; buffer
    mov edx, ecx ; port
    mov ecx, [esp + 8] ; size
    shr ecx, 1
    cld
    rep outsw
    pop esi
    ret

; u8 rotl_byte(u8 value, u8 count)
global rotl_byte
rotl_byte:
    mov al, cl ; value
    mov cl, dl ; count
    rol al, cl
    ret 

; u8 rotr_byte(u8 value, u8 count)
global rotr_byte
rotr_byte:
    mov al, cl ; value
    mov cl, dl ; count
    ror al, cl
    ret

; u32 read_eflags()
global read_eflags
read_eflags:
    pushf
    pop eax
    ret

; void write_eflags(u32 eflags);
global write_eflags
write_eflags:
    push eax
    popf
    ret 