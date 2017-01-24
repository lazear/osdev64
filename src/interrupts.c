/*
uart.c
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

#include <interrupts.h>
#include <desc.h>
#include <common.h>
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

void breakpoint_handler(struct registers* r)
{
	printf("Breakpoint RIP %#x RFLAGS %x\n", r->rip, r->flags);
	printf("rax %#x rbx %#x\n", r->rax, r->rbx);
	printf("rcx %#x rdx %#x\n", r->rcx, r->rdx);
	printf("rsi %#x rdi %#x\n", r->rsi, r->rdi);
	printf("rsp %#x rbp %#x\n", r->rsp, r->rbp);
	printf("r8  %#x r9  %#x\n", r->r8, r->r9);
	printf("r10 %#x r11 %#x\n", r->r10, r->r11);
	printf("r12 %#x r13 %#x\n", r->r12, r->r13);
	printf("r14 %#x r15 %#x\n", r->r14, r->r15);
}

void trap_register(int num, void (*handler)(struct registers*))
{	
	if (num < 256)
		handlers[num] = handler;
}


void trap(struct registers* r)
{
	if (handlers[r->int_no]) {
		handlers[r->int_no](r);

	} else if (r->int_no < IRQ_ZERO) {
		/* CPU exception without a handler installed */
		char buf[100];
		snprintf(buf, 100, "CPU exception: %s\n", exceptions[r->int_no]);
		vga_puts(buf);

		halt_catch_fire();
	} else {
		// char buf[100];
		// snprintf(buf, 100, "Unknown interrupt: %d\n", r->int_no);
		// vga_puts(buf);
	}
	/* send EOI to slave PIC */
	if (r->int_no > 40)
		outb(0xA0, 0x20);
	outb(0x20, 0x20);
}


void trap_init(void)
{
	memset(handlers, 0, sizeof(trap_handler)*256);
	trap_register(3, breakpoint_handler);
}