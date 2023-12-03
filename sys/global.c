#include <sys/type.h>
#include <sys/const.h>
#include <sys/protect.h>
#include <sys/proto.h>
#include <sys/global.h>

u8 gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
descriptor_t gdt[GDT_SIZE];
u8 idt_ptr[6];	/* 0~15:Limit  16~47:Base */
gate_t idt[IDT_SIZE];
tss_t tss;