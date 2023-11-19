#include "kernel/type.h"
#include "kernel/const.h"
#include "kernel/protect.h"
#include "kernel/proto.h"
#include "kernel/global.h"

u8 gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
DESCRIPTOR gdt[GDT_SIZE];
u8 idt_ptr[6];	/* 0~15:Limit  16~47:Base */
GATE idt[IDT_SIZE];