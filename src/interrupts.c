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


void trap(struct registers* r)
{
	if (r->int_no < 0x20) {
		asm volatile("cli");
		/* safe printing... */
		char buf[100];
		snprintf(buf, 100, "CPU exception: %s\n", exceptions[r->int_no]);
		vga_puts(buf);

		halt_catch_fire();
	}

	outb(0x20, 0x20);
}