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

#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/desc.h>
#include <arch/x86_64/kernel.h>
#include <arch/x86_64/mmu.h>
#include <lapic.h>
#include <stack_trace.h>
#include <stdint.h>
#include <stdio.h>


extern void halt_catch_fire(void);

static trap_handler handlers[256];

static char* exceptions[] = {
	"Divide by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor",
	"Invalid TSS",
	"Segment not present",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 FPU",
	"Alignment check",
	"Machine check",
	"SIMD FP",
	"Virtualization fault",
	"Reserved",
	[0x1E] = "Security fault",
};

static void dump_registers(struct registers* r)
{
	printf("Core dump\n");
	printf("rip %#x rflags %x\n", r->rip, r->flags);
	printf("cs  %x ss  %x\n", r->cs, r->ss);
	printf("rax %#x rbx %#x\n", r->rax, r->rbx);
	printf("rcx %#x rdx %#x\n", r->rcx, r->rdx);
	printf("rsi %#x rdi %#x\n", r->rsi, r->rdi);
	printf("rsp %#x rbp %#x\n", r->rsp, r->rbp);
	printf("r8  %#x r9  %#x\n", r->r8, r->r9);
	printf("r10 %#x r11 %#x\n", r->r10, r->r11);
	printf("r12 %#x r13 %#x\n", r->r12, r->r13);
	printf("r14 %#x r15 %#x\n", r->r14, r->r15);
}

int breakpoint_handler(struct registers* r)
{
	dump_registers(r);
	if (r->rax == 0)
		halt_catch_fire();
	return 0;
}

int page_fault(struct registers* r) 
{
	size_t cr2;
	asm volatile("mov %%cr2, %%rax" : "=a"(cr2));

	printf("Page Fault(%2d)   cr2:  %#x\n", r->err_no, cr2);
	printf("Faulting instruction:  %#x (%s)\n", r->rip, (r->cs == 0x23) ? "User process" : "Kernel code");
	printf("Current stack pointer: %#x/%#x\n", r->rsp, r->rbp);
	

	if (r->err_no & PRESENT)	printf(" \tPage Not Present\n");
	if (r->err_no & RW) 		printf(" \tPage Not Writeable\n");
	if (r->err_no & USER) 		printf(" \tPage Supervisor Mode\n");

	//if (r->cs == 0x23)
	printf("Beginning stack trace:\n");
	stack_trace(r->rbp);

	asm volatile("mov %%rax, %%cr2" :: "a"(0x00000000));
	halt_catch_fire();
	return 0;
}


void trap_register(int num, int (*handler)(struct registers*))
{	
	if (num < 256)
		handlers[num] = handler;
	else {
		printf("panic: Trying to install a handler for interrupt %d\n", num);
		halt_catch_fire();
	}
}


void trap(struct registers* r)
{
	if (handlers[r->int_no]) {
		int err = handlers[r->int_no](r);
		if (err) {
			printf("Trap handler 0x%x returned value %#x\n", r->int_no, err);
			dump_registers(r);
			halt_catch_fire();
		}
	} else if (r->int_no < IRQ_ZERO) {
		/* CPU exception without a handler installed */
		printf("CPU exception %s\n", exceptions[r->int_no]);
		dump_registers(r);		
		stack_trace(r->rbp);
		halt_catch_fire();
	} else {
		/* Non-exception without handler installed */
		printf("Interrupt 0x%x without handler\n", r->int_no);
		dump_registers(r);
		stack_trace(r->rbp);
		halt_catch_fire();
	}
	/* send EOI to slave PIC */
	if (r->int_no > 40)
		outb(0xA0, 0x20);

	if (lapic_active())
		lapic_eoi();

	outb(0x20, 0x20);
}


void trap_init(void)
{
	memset(handlers, 0, sizeof(trap_handler)*256);
	trap_register(BREAKPOINT, breakpoint_handler);
	trap_register(PAGEFAULT, page_fault);
}