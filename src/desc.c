#include <stddef.h>
#include <stdint.h>

#include "x86_64.h"



void idt_set_gate(int n, int dpl, int type, uint64_t handler) 
{
	idt[n].offset_low 	= handler & 0xFFFF;
	idt[n].offset_mid 	= (handler >> 16) & 0xFFFF;
	idt[n].offset_high	= (handler >> 32);

	idt[n].segment_selector = 0x8;
	idt[n].reserved = 0;
	/*
	type[0:2] IST
	type[3:7] 0
	type[8:B] type flags
	type[C:C] 0
	type[D:E] DPL
	type[F:F] present
	*/
	idt[n].type = ((1 << 0xF) | (dpl << 0xD) | (type << 0x8));
}



void gdt_init(void)
{

	/* It's easier to just do this by hand than to use structs for 64 bit GDT */
	memset(gdt, 0, sizeof(gdt));
	gdt[0] = 0;
	gdt[1] = 0x00209A0000000000; 	/* 0x08 KERNEL_CODE */
	gdt[2] = 0x0000920000000000; 	/* 0x10 KERNEL_DATA */
	gdt[3] = 0x0020FA0000000000; 	/* 0x18 USER_CODE */
	gdt[4] = 0x0000F20000000000; 	/* 0x20 USER_DATA */
	gdt[5] = 0x0000920000000000; 	/* 0x28 KERNEL_CPU */

	/* According to the Intel manuals, the 64 bit TSS 
	 * segment descriptor now takes up 16 bytes, so load it
	 * into the 6th and 7th entries */
	uint64_t tss = 0x0000890000000000;
	tss |= (((uint64_t) &system_tss) & 0xFFFF) << 16;
	tss |= ((((uint64_t) &system_tss) >> 16) & 0xFF) << 24;
	tss |= ((((uint64_t) &system_tss) >> 24) & 0xFF) << 56;
	tss |= sizeof(system_tss) - 1;

	gdt[6] = tss;					/* 0x30 KERNEL_TSS */
	gdt[7] = ((uint64_t) &system_tss) >> 32;

	/* 10 byte GDT descriptor */
	uint8_t gdtr[10];
	*(uint16_t*) gdtr = (uint16_t) sizeof(gdt) -1;
	*(uint64_t*) ((uint64_t) gdtr + 2) = (uint64_t) &gdt;



	gdt_flush((uint64_t) gdtr);
}


void idt_init(void)
{
	int i;
	for (i = 0; i < 256; i++)
		idt_set_gate(i, 0, 0xE, (uint64_t) vectors[i]);

	uint8_t idtr[10];
	*(uint16_t*) idtr = (uint16_t) 0xFFF;
	*(uint64_t*) ((uint64_t) idtr + 2) = (uint64_t) &idt;

	system_tss.ist[0] = 0xFFFF80000000A000;
	system_tss.rsp[0] = 0xFFFF80000000A000;
	//printf('IST0: %x\n', system_tss.ist[0]);
	asm volatile("lidt (%0)" : : "r" ((uint64_t)&idtr));

}