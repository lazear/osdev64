/*
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
*/

#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/kernel.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/mmu.h>
#include <arch/x86_64/desc.h>
#include <sysconfig.h>
#include <frame.h>
#include <stdio.h>
#include <vga.h>
#include <sse.h>
#include <acpi.h>

extern int x2apic_enabled(void);
extern int printf(const char* fmt, ...);

struct memory_map
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
};

void pmm_init(size_t mem_map_location, int mem_map_length)
{
	kernel_log("[init] parsing memory map\n");
	/* Parse memory map */

	struct memory_map* mmap = (struct memory_map*) mem_map_location;
	struct memory_map* mmax = mmap + mem_map_length;
	
	size_t highest_addr = 0;
	while (mmap < mmax) {
		size_t start = mmap->base;
		size_t end = mmap->base+mmap->len;
		kernel_log("[init] %8s memory detected %#x - %#x\n",(mmap->type & 0xF) == 1 ? "Usable" : "Reserved", start, end);
		if ((mmap->type & 0xF) == 1) {
			highest_addr = mmap->len + mmap->base;
		}
		mmap++;
	}
	page_init(highest_addr, V2P(KERNEL_END));

	mmap = (struct memory_map*) P2V(0x7000);
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
}


void main(void)
{
	int sse = sse42_enabled();
	config_set(CONFIG_SSE, sse);
	if (! sse) {
		/* SSE4.2 is not heavily used by the kernel... yet.
		 * So we will not worry too much */
		kernel_log("[init] No SSE4.2 support!\n");
	}
	vga_clear();
	gdt_init(); 	
	idt_init();			
	pic_init();
	trap_init();
	syscall_init();

	uint8_t memlen = *(uint8_t*) P2V(0x6FF0) / sizeof(struct memory_map);
	pmm_init(P2V(0x7000), memlen);
	mmu_init();

	printf("[init] early kernel boot success!\n");
	printf("[init] %d processors detected\n", acpi_init());

	pic_disable();
	lapic_init();
	printf("Everything is good to go!\n");
//	sti();
	for(;;);
}
