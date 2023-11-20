#include <kernel/protect.h>
#include <kernel/global.h>
#include <lib/display.h>
#include <lib/string.h>

char pusheen_os[] = {
" ____  __  __  ___  _   _  ____  ____  _  _    _____  ___ \n"
"(  _ \\(  )(  )/ __)( )_( )( ___)( ___)( \\( )  (  _  )/ __)\n"
" )___/ )(__)( \\__ \\ ) _ (  )__)  )__)  )  (    )(_)( \\__ \\\n"
"(__)  (______)(___/(_) (_)(____)(____)(_)\\_)  (_____)(___/\n"
};

void cstart()
{
	disp_reset();
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                 "-----\"cstart\" begins-----\n");

	/* 将 LOADER 中的 GDT 复制到新的 GDT 中 */
	memcpy(&gdt, (void*)(*((u32*)(&gdt_ptr[2]))), *((u16*)(&gdt_ptr[0])) + 1);

	/* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
	u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(descriptor_t) - 1;
	*p_gdt_base  = (u32)&gdt;

	/* idt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sidt/lidt 的参数。*/
	u16* p_idt_limit = (u16*)(&idt_ptr[0]);
	u32* p_idt_base  = (u32*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(gate_t) - 1;
	*p_idt_base  = (u32)&idt;

	init_prot();

	disp_str("-----\"cstart\" ends-----\n");

	disp_clearlines(5);
	disp_reset();
	disp_color_str(pusheen_os, 0xE);
}
