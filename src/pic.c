/*
pic.c
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

#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/kernel.h>
#include <stdint.h>

#define PIC1_CMD		0x20
#define PIC1_DATA		0x21
#define PIC2_CMD		0xA0
#define PIC2_DATA		0xA1

#define IRQ_SLAVE       2       /* IRQ at which slave connects to master */


/* Initial IRQ mask has interrupt 2 enabled (for slave 8259A).*/
static uint16_t irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);

/* Mask interrupts on the PIC */
static void pic_setmask(uint16_t mask) 
{
	irqmask = mask;
	outb(PIC1_DATA, mask);
	outb(PIC2_DATA, mask >> 8);
}

void pic_enable(int irq) 
{
 	pic_setmask(irqmask & ~(1<<irq));
}

/* Mask all interrupts */
void pic_disable(void) 
{
	pic_setmask(0xFFFF);
}

/* Initialize the 8259A interrupt controllers. */
void pic_init(void) 
{
	/* mask all interrupts */
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	/* Set up master (8259A-1) */
	outb(PIC1_CMD, 0x11);

	/* ICW2:  Vector offset */
	outb(PIC1_DATA, IRQ_ZERO);
	outb(PIC1_DATA, 1<<IRQ_SLAVE);

	outb(PIC1_DATA, 0x3);

	/* Set up slave (8259A-2) */
	outb(PIC2_CMD, 0x11);
	outb(PIC2_DATA, IRQ_ZERO + 8); 
	outb(PIC2_DATA, IRQ_SLAVE);
	outb(PIC2_DATA, 0x3);

	outb(PIC1_CMD, 0x68);
	outb(PIC1_CMD, 0x0a);

	outb(PIC2_CMD, 0x68);
	outb(PIC2_CMD, 0x0a);

	if(irqmask != 0xFFFF)
		pic_setmask(irqmask);
}