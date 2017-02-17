/*
MIT License
Copyright (c) 2016-2017 Michael Lazear

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
*/

// /*
// http://www.intel.com/design/pentium/datashts/24201606.pdf

// BSP: Bootstrap processor - the core we are currently running on
// AP: Application processor(s). Currently halted.
// */


// #include <stdint.h>
// #include <stddef.h>
// #include <frame.h>
// #include <lock.h>
// #include <lapic.h>
// #include <arch/x86_64/kernel.h>
// #include <arch/x86_64/cpu.h>
// #include <arch/x86_64/desc.h>
// #include <arch/x86_64/mmu.h>
// #include <arch/x86_64/interrupts.h>


// extern size_t _binary_ap_entry_start[];
// extern size_t _binary_ap_entry_end[];

// volatile int ncpu = 0;

// static struct lock lock;
// static int lapic_up = 0;


// static void lapic_write(int index, int value) 
// {
// 	*(size_t*) (LAPIC_BASE + index) = value;	/* Write to lapic register */
// 	*(size_t*) (LAPIC_BASE + LAPIC_ID);	/* Wait by reading */
// }

// static size_t lapic_read(int index) 
// {
// 	return *(size_t*) (LAPIC_BASE + index);
// }

// int lapic_active()
// {
// 	return lapic_up;
// }

// void lapic_eoi() 
// {
// 	if (!lapic_up)
// 		return;
// 	lapic_write(LAPIC_EOI, 0);
// }

// void lapic_test(uint16_t dest, uint16_t sh, uint16_t vector) 
// {
// 	lapic_write(LAPIC_ICRHI, dest << 24);
// 	lapic_write(LAPIC_ICRLO, (sh << 17) | vector);
// }

// void lapic_timer_config(uint8_t mode, uint32_t initial_count, uint8_t divide_by) 
// {
// 	/* LVT Timer register:
// 		19:17 Timer mode
// 		16:15 Mask
// 		12:11 Delivery status
// 		7:0 Vector 	*/
// 	lapic_write(TIMER, (mode << 17) | 0x20 + IRQ_TIMER);
// 	/* LVT Divide Configuration Register, bits 0, 1, and 3 
// 		0000 Divide by 2
// 		0001 Divide by 4
// 		0010 Divide by 8
// 		0011 Divide by 16
// 		1000 Divide by 32
// 		1001 Divide by 64
// 		1010 Divide by 128
// 		1011 Divide by 1 */
// 	lapic_write(DIVIDE_CONF, divide_by);
// 	lapic_write(INIT_COUNT, initial_count);
// 	lapic_write(CURR_COUNT, 0);
// 	kernel_log("[lapic] cpu %d timer mode set to %d, initial count: %#x, divide_by: %X\n", lapic_read(LAPIC_ID)>>24, mode, initial_count, divide_by);
// }

// int lapic_danger_id() 
// {
// 	return lapic_read(LAPIC_ID) >> 24;
// }

// /* Initilize the local advanced programmable interrupt chip 
// PIC should already be disabled by the time we get here*/
// void lapic_init() {
// 	lock_init(&lock);

// 	struct page lapic_base;
// 	lapic_base.address = LAPIC_BASE;
// 	mmu_map_page(&lapic_base, LAPIC_BASE, PRESENT | RW);

// 	/* Enable local APIC and set the spurious interrupt vector */
// 	lapic_write(LAPIC_SIV, 0x100 | (IRQ_ZERO + IRQ_SPURIOUS));

// 	/* Setup timer on the first CPU only to avoid race for timer()*/
// 	lapic_timer_config(PERIODIC, 0x10000, 0x0A);


// 	/* Mask local interrupts */
// 	//lapic_write(LAPIC_LINT0, 0x10000);
// 	//lapic_write(LAPIC_LINT1, 0x10000);

// 	if ((lapic_read(LAPIC_VER) >> 16) & 0xFF >= 4)
// 		lapic_write(0x0340, 0x10000);

// 	lapic_write(LAPIC_ERR, IRQ_ZERO + IRQ_ERROR);

// 	lapic_write(LAPIC_ERR, 0);
// 	lapic_write(LAPIC_EOI, 0);	// Clear any existing interrupts

// 	lapic_write(LAPIC_ICRHI, 0);
// 	lapic_write(LAPIC_ICRLO, INIT | LEVEL | BCAST);

// 	while(lapic_read(LAPIC_ICRLO) & DELIVS)
// 		;

// 	lapic_write(LAPIC_TPR, 0);
// 	lapic_up = 1;
// }

// int udelay(int i) {
// 	lapic_read(LAPIC_ID);
// }

// /* Sends a start up IPI to the AP processor designated <apic_id>,
// telling it to start executing code at <address> 
// Address must be page-aligned, and in the first megabyte of system mem*/
// void lapic_start_AP(int apic_id, uint32_t address) {

// 	/* Following Intel SMP startup procedure:
// 	Write 0Ah to CMOS RAM location 0Fh (shutdown code). 
// 	This initializes a warm reset
// 	*/
// 	outb(0x70, 0xF);
// 	outb(0x71, 0xA);

// 	/*
// 	The STARTUP IPI causes the target processor to start executing in Real Mode from address
// 	000VV000h, where VV is an 8-bit vector that is part of the IPI message. Startup vectors are
// 	limited to a 4-kilobyte page boundary in the first megabyte of the address space. Vectors A0-BF
// 	are reserved; do not use vectors in this range. 
// 	*/
// 	uint16_t* warm_reset_vector = (uint16_t*) P2V(0x40 << 4 | 0x67);
// 	warm_reset_vector[0] = 0;
// 	warm_reset_vector[1] = address >> 4;
// 	//*warm_reset_vector = address;

// 	lapic_write(LAPIC_ICRHI, apic_id << 24);
// 	lapic_write(LAPIC_ICRLO, INIT | LEVEL | ASSERT);
// 	udelay(2);
// 	lapic_write(LAPIC_ICRLO, INIT | LEVEL);
// 	udelay(1);

// 	/* Keep this printf here, it acts as a delay... lol */
// 	kernel_log("[lapic] starting cpu: %d\n", apic_id);
// 	for (int i = 0; i < 2; i++) {
// 		lapic_write(LAPIC_ICRHI, apic_id << 24);
// 		lapic_write(LAPIC_ICRLO, STARTUP |  address >> 12);
// 		udelay(1);
// 	}

// }




// void mp_enter() {
// 	lock_acquire(&lock);
// 	/* What's our APIC id? */
// 	int id = lapic_read(LAPIC_ID) >> 24;

// 	ncpu++;
// 	/* Initialize GDT/IDT for cpu n */
// 	gdt_init_cpu(id);		
	
// 	uint8_t idtr[10];
// 	*(uint16_t*) idtr = (uint16_t) 0xFFF;
// 	*(uint64_t*) ((uint64_t) idtr + 2) = (uint64_t) &idt;


// 	asm volatile("lidt (%0)" : : "r" ((uint64_t)&idtr));

// 	/* Store id in cpu-local variable */
// 	cpu->id = id;
// 	/* Initialize the processors's LAPIC */
// 	lapic_init();

// 	/* Release spinlock, allowing next processor in */
// 	lock_release(&lock);
// }

// int mp_processor_id() {
// 	//if interrupts enabled, panic
// 	if (get_eflags() & 0x200) {
// 		panic("INTERRUPTS ENABLED");
// 		return -1;
// 	}
// 	return lapic_read(LAPIC_ID) >> 24;
// }


// int mp_number_of_processors() {
// 	return ncpu + 1;	
// }



// void mp_start_ap(int nproc) {

// 	int ap_entry_size = (size_t) _binary_ap_entry_end - (size_t) _binary_ap_entry_start;
// 	uint8_t* code = (uint8_t*) 0x8000;
// 	memcpy(code, _binary_ap_entry_start, ap_entry_size);

// 	for (int i = 1; i < nproc; i++) {

// 		struct page* p = page_alloc();

// 		cpus[i].stack =  p->address;
// 		cpus[i].id = i;
// 		cpus[i].cpu = &cpus[i];

// 		/*		
// 		 * 0x8000 execution point
// 		 * 0x7FF8 page directory 
// 		 * 0x7FF0 stack (32 bit location)
// 		 * 0x7FE8 entry function
// 		 * 0x7FE0 stack (64 bit)
// 		 */

// 		*(size_t**)(code - 0x08) = (void*) cpus[i].stack;
// 		*(size_t**)(code - 0x10) = (void*) mp_enter;
// 		*(size_t**)(code - 0x18) = (size_t*) get_cr3();
		
// 		lapic_start_AP(i, 0x8000);

// 	}

// }