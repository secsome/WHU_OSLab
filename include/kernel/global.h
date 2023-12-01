#pragma once

#include <kernel/const.h>
#include <kernel/type.h>
#include <kernel/protect.h>
#include <kernel/proc.h>

HEADER_CPP_BEGIN

extern u8               gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
extern descriptor_t     gdt[GDT_SIZE];
extern u8               idt_ptr[6];	/* 0~15:Limit  16~47:Base */
extern gate_t           idt[IDT_SIZE];
extern tss_t            tss;

HEADER_CPP_END