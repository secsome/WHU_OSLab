#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/protect.h>

EXTERN	u8              gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	descriptor_t    gdt[GDT_SIZE];
EXTERN	u8              idt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	gate_t          idt[IDT_SIZE];
