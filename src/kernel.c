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

#include <stdint.h>
#include <stddef.h>
#include <common.h>
#include <interrupts.h>
#include <frame.h>
#include <mmu.h>
#include <desc.h>
#include <stdio.h>
#include <vga.h>
#include <sse.h>


extern int x2apic_enabled(void);
extern int printf(const char* fmt, ...);

struct memory_map
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
};

extern void mmu_map1gb(size_t physical, size_t address, int flags);

void main(void)
{
	gdt_init(); 	
	idt_init();			
	pic_init();
	trap_init();
	syscall_init();
	vga_clear();
	printf("Control transferred to x86_64 kernel\n");
	dprintf("[init] early kernel boot success!\n");


	dprintf("[init] parsing memory map\n");
	/* Parse memory map */
	uint8_t memlen = *(uint8_t*) 0x6FF0 / sizeof(struct memory_map);
	struct memory_map* mmap = (struct memory_map*) 0x7000;
	struct memory_map* mmax = mmap + memlen;
	
	size_t highest_addr = 0;
	while (mmap < mmax) {
		size_t start = mmap->base;
		size_t end = mmap->base+mmap->len;
		dprintf("[init] %8s memory detected %#x - %#x\n",(mmap->type & 0xF) == 1 ? "Usable" : "Reserved", start, end);
		if ((mmap->type & 0xF) == 1) {
			highest_addr = mmap->len + mmap->base;
		}
		mmap++;
	}
	page_init(highest_addr, V2P(KERNEL_END));

	mmap = (struct memory_map*) 0x7000;
	while (mmap < mmax) {
		if (mmap->base+mmap->len <= highest_addr) {
			if ((mmap->type & 0xF) == 1) {
				page_mark_range(mmap->base, mmap->base+mmap->len, P_FREE);
			} else {
				page_mark_range(mmap->base, mmap->base+mmap->len, P_USED);
			}
		}

		mmap++;
	}
	/* Mark everything below end of kernel as belonging to us */
	page_mark_range(0, V2P(KERNEL_END), P_KERNEL|P_IMMUTABLE|P_USED);
	/* Physical memory manager is now up and running */

	mmu_init();

	dprintf("[init] probing processor features\n");
	//printf("%dM total physical memory detected\n", phys_mem_detected / 0x100000);
	printf("x2APIC? %d\n", x2apic_enabled() & (1<<21));
	printf("SSE4.2? %d\n", sse42_enabled());

	for(;;);
}
