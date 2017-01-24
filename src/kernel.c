#include <stdint.h>
#include <stddef.h>
#include <common.h>
/*
kernel.c
===============================================================================
MIT License
Copyright (c) Michael Lazear 2016-2017 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/

#include <desc.h>
#include <stdio.h>
#include <vga.h>
#include <interrupts.h>
#include <buddy.h>
#include <setjmp.h>

extern int x2apic_enabled(void);
extern int printf(const char* fmt, ...);

struct memory_map
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
};

extern int sse42_enabled(void);
extern char* sse_strchr(const char* s, char c);


void t9(void)
{
	printf("jmp");
	halt_catch_fire();
}

char buf[100];
void main(void)
{
	gdt_init();
	idt_init();
	pic_init();
	trap_init();
	vga_clear();

	uint8_t memlen = *(uint8_t*) 0x6FF0 / sizeof(struct memory_map);
	struct memory_map* mmap = (struct memory_map*) 0x7000;
	struct memory_map* mmax = mmap + memlen;
	printf("Control transferred to x86_64 kernel\n");

	buddy_initialize(0x80000000, 0xffff800000200000, 0x4000);
	uint64_t phys_mem_detected = 0;
	while (mmap < mmax) {
		//printf("%8s memory detected %#x - %#x\n",(mmap->type & 0xF) == 1 ? "Usable" : "Reserved", mmap->base, mmap->base + mmap->len);
		if ((mmap->type & 0xF) == 1) {
			struct buddy* a = buddy_add_range(mmap->base, mmap->base+mmap->len);
			printf("INIT addr %#x - %#x\n", a->addr, a->addr + (1 << a->size));
			phys_mem_detected += mmap->len;
		}
		mmap++;
	}

	printf("%dM total physical memory detected\n", phys_mem_detected / 0x100000);
	printf("x2APIC? %d %x\n", x2apic_enabled(), readmsr(0x1B) & (1<<8));
	printf("SSE4.2? %d\n", sse42_enabled());
	//printf("strchr %s\n", sse_strchr("The quick brown fox jumped over the lazy dog", 'j'));
	// pic_enable(32);
	// syscall_init();

	printf("Before jmp\n");
	struct jmp_buf j;
	uint64_t x = setjmp(&j);

	printf("After jmp %x\n", x);
	if (x != 10)
		longjmp(&j, 10);
	if (x == 10) {
		j.rip = t9;
		longjmp(&j, 12);
	}



	for(;;);
}
