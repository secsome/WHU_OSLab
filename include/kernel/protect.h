#pragma once

#include <kernel/const.h>
#include <kernel/type.h>

HEADER_CPP_BEGIN

// 存储段描述符/系统段描述符
typedef struct descriptor_t
{
	u16	limit_low;		/* Limit */
	u16	base_low;		/* Base */
	u8	base_mid;		/* Base */
	u8	attr1;			/* P(1) DPL(2) DT(1) TYPE(4) */
	u8	limit_high_attr2;	/* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	u8	base_high;		/* Base */
} descriptor_t;

// 门描述符
typedef struct gate_t
{
	u16	offset_low;	/* Offset Low */
	u16	selector;	/* Selector */
	u8	dcount;		/* 该字段只在调用门描述符中有效。如果在利用
				   调用门调用子程序时引起特权级的转换和堆栈
				   的改变，需要将外层堆栈中的参数复制到内层
				   堆栈。该双字计数字段就是用于说明这种情况
				   发生时，要复制的双字参数的数量。*/
	u8	attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	u16	offset_high;	/* Offset High */
} gate_t;

typedef struct tss_t 
{
	u32	backlink;
	u32	esp0;		/* stack pointer to use during interrupt */
	u32	ss0;		/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	// I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图
	// u8	iomap[2];
} tss_t;

u32 seg2phys(u16 seg);
#define vir2phys(seg_base, vir)	(u32)(((u32)seg_base) + (u32)(vir))

// Hardware irqs
enum
{
	NUM_IRQ = 16,
	CLOCK_IRQ = 0,
	KEYBOARD_IRQ = 1,
	CASCADE_IRQ = 2,	// cascade enable for 2nd AT controller
	ETHER_IRQ = 3,		// default ethernet interrupt vector
	SECONDARY_IRQ = 3,	// RS232 interrupt vector for port 2
	RS232_IRQ = 4,		// RS232 interrupt vector for port 1
	XT_WINI_IRQ = 5,	// xt winchester
	FLOPPY_IRQ = 6,		// floppy disk 
	PRINTER_IRQ = 7,
	AT_WINI_IRQ = 14	// at winchester
};

typedef void (*irq_handler)(int irq);
extern irq_handler irq_table[NUM_IRQ];

void put_irq_handler(int irq, irq_handler handler);
void enable_irq(int irq);
void disable_irq(int irq);

// GDT
// 描述符索引
enum
{
	INDEX_DUMMY = 0,
	INDEX_FLAT_C = 1,
	INDEX_FLAT_RW = 2,
	INDEX_VIDEO = 3,
	INDEX_TSS = 4,
	INDEX_LDT_FIRST = 5
};

// 选择子
enum
{
	SELECTOR_DUMMY = 0,
	SELECTOR_FLAT_C = 0x8,
	SELECTOR_FLAT_RW = 0x10,
	SELECTOR_VIDEO = 0x18 + 3,
	SELECTOR_TSS = 0x20, // TSS. 从外层跳到内存时 SS 和 ESP 的值从里面获得.
	SELECTOR_LDT_FIRST = 0x28
};

enum
{
	SELECTOR_KERNEL_CS = SELECTOR_FLAT_C,
	SELECTOR_KERNEL_DS = SELECTOR_FLAT_RW,
	SELECTOR_KERNEL_GS = SELECTOR_VIDEO
};

// 每个任务有一个单独的 LDT, 每个 LDT 中的描述符个数
enum 
{
	LDT_SIZE = 2
};

enum
{
	// 描述符类型值说明
	DA_32 = 0x4000,			// 32 位段
	DA_LIMIT_4K = 0x8000,	// 段界限粒度为 4K 字节
	DA_DPL0 = 0x00,			// DPL = 0
	DA_DPL1 = 0x20,			// DPL = 1
	DA_DPL2 = 0x40,			// DPL = 2
	DA_DPL3 = 0x60,			// DPL = 3
	// 存储段描述符类型值说明
	DA_DR = 0x90,			// 存在的只读数据段类型值
	DA_DRW = 0x92,			// 存在的可读写数据段属性值
	DA_DRWA = 0x93,			// 存在的已访问可读写数据段类型值
	DA_C = 0x98,			// 存在的只执行代码段属性值
	DA_CR = 0x9A,			// 存在的可执行可读代码段属性值
	DA_CCO = 0x9C,			// 存在的只执行一致代码段属性值
	DA_CCOR = 0x9E,			// 存在的可执行可读一致代码段属性值
	// 系统段描述符类型值说明
	DA_LDT = 0x82,			// 局部描述符表段类型值
	DA_TaskGate = 0x85,		// 任务门类型值
	DA_386TSS = 0x89,		// 可用 386 任务状态段类型值
	DA_386CGate = 0x8C,		// 386 调用门类型值
	DA_386IGate = 0x8E,		// 386 中断门类型值
	DA_386TGate = 0x8F,		// 386 陷阱门类型值
};

// 选择子类型值说明
// 其中, SA_ : Selector Attribute
enum 
{
	SA_RPL_MASK = 0xFFFC,
	SA_RPL0	= 0,
	SA_RPL1	= 1,
	SA_RPL2	= 2,
	SA_RPL3	= 3,
	SA_TI_MASK = 0xFFFB,
	SA_TIG = 0,
	SA_TIL = 4,
	RPL_KRNL = SA_RPL0,
	RPL_TASK = SA_RPL1,
	RPL_USER = SA_RPL3
};

// 中断向量
enum 
{
	INT_VECTOR_DIVIDE = 0x0,
	INT_VECTOR_DEBUG = 0x1,
	INT_VECTOR_NMI = 0x2,
	INT_VECTOR_BREAKPOINT = 0x3,
	INT_VECTOR_OVERFLOW = 0x4,
	INT_VECTOR_BOUNDS = 0x5,
	INT_VECTOR_INVAL_OP = 0x6,
	INT_VECTOR_COPROC_NOT = 0x7,
	INT_VECTOR_DOUBLE_FAULT = 0x8,
	INT_VECTOR_COPROC_SEG = 0x9,
	INT_VECTOR_INVAL_TSS = 0xA,
	INT_VECTOR_SEG_NOT = 0xB,
	INT_VECTOR_STACK_FAULT = 0xC,
	INT_VECTOR_PROTECTION = 0xD,
	INT_VECTOR_PAGE_FAULT = 0xE,
	INT_VECTOR_COPROC_ERR = 0x10,
	INT_VECTOR_IRQ0 = 0x20,
	INT_VECTOR_IRQ8 = 0x28,
};

HEADER_CPP_END