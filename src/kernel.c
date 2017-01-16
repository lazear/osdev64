#include <stdint.h>
#include <stddef.h>
#include <common.h>
#include <desc.h>
#include <stdio.h>
#include <vga.h>
#include <interrupts.h>

extern int x2apic_enabled(void);
extern int printf(const char* fmt, ...);

struct memory_map
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
};


char buf[100];
void main(void)
{
	gdt_init();
	idt_init();
	pic_init();

	vga_clear();

	uint8_t memlen = *(uint8_t*) 0x6FF0 / sizeof(struct memory_map);
	struct memory_map* mmap = (struct memory_map*) 0x7000;
	struct memory_map* mmax = mmap + memlen;
	printf("Control transferred to x86_64 kernel\n");


	uint64_t phys_mem_detected = 0;
	while (mmap < mmax) {
		printf("%8s memory detected %#x - %#x\n",(mmap->type & 0xF) == 1 ? "Usable" : "Reserved", mmap->base, mmap->base + mmap->len);
		if ((mmap->type & 0xF) == 1)
			phys_mem_detected += mmap->len;
		mmap++;
	}

	printf("%dM total physical memory detected\n", phys_mem_detected / 0x100000);
	printf("x2APIC? %d %x\n", x2apic_enabled(), readmsr(0x1B) & (1<<8));

	sti();

	pic_enable(32);
	syscall_config();
	asm volatile("syscall");
	for(;;);
}
