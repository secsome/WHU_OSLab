#include <sys/protect.h>
#include <lib/display.h>
#include <lib/asm.h>

enum
{
	INT_M_CTL = 0x20,		// I/O port for interrupt controller       <Master>
	INT_M_CTLMASK = 0x21,	// setting bits in this port disables ints <Master>
	INT_S_CTL = 0xA0,		// I/O port for second interrupt controller<Slave>
	INT_S_CTLMASK = 0xA1	// setting bits in this port disables ints <Slave>
};

void fake_irq(int irq)
{
    disp_str("fake_irq: ");
    disp_int(irq);
    disp_str("\n");
}

void enable_irq(int irq)
{
	if (irq < 8)
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
	else if (irq < 16)
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << (irq - 8)));
}

void disable_irq(int irq)
{
	if (irq < 8)
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
	else if (irq < 16)
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << (irq - 8)));
}

void put_irq_handler(int irq, irq_handler handler)
{
	disable_irq(irq);
	irq_table[irq] = handler;
}

void init_8259A()
{
	out_byte(INT_M_CTL,	0x11);			// Master 8259, ICW1.
	out_byte(INT_S_CTL,	0x11);			// Slave  8259, ICW1.
	out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	// Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20.
	out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	// Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28
	out_byte(INT_M_CTLMASK,	0x4);			// Master 8259, ICW3. IR2 对应 '从8259'.
	out_byte(INT_S_CTLMASK,	0x2);			// Slave  8259, ICW3. 对应 '主8259' 的 IR2.
	out_byte(INT_M_CTLMASK,	0x1);			// Master 8259, ICW4.
	out_byte(INT_S_CTLMASK,	0x1);			// Slave  8259, ICW4.

	out_byte(INT_M_CTLMASK,	0xFF);	// Master 8259, OCW1. 
	out_byte(INT_S_CTLMASK,	0xFF);	// Slave  8259, OCW1. 

	for (auto& irq : irq_table)
		irq = fake_irq;
}
