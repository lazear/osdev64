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

#include <stdint.h>
#include <common.h>
#include <drivers/uart.h>

void uart_init() 
{
	outb(COM1 + 1, 0x00);	/* Int enable register, disable interupts */
	outb(COM1 + 3, 0x80);	/* Enable baud rate divisor */
	outb(COM1 + 0, 0x03);	/* Data register, divide by 3. (1152000 / 3 = 38400 baud) */
	outb(COM1 + 1, 0x00);	/* hi-byte */
	outb(COM1 + 3, 0x03);	/* Lock divisor, 8 data bits, no parity */
	outb(COM1 + 2, 0xC7);	/* Enable FIFO, clear them, 14-byte threshold */
	outb(COM1 + 4, 0x00);	/* Interrupts disabled */
}

void uart_putc(char c) 
{
	while((inb(COM1 + 5) & 0x20) == 0);
	outb(COM1, c);
}

void uart_write(const char* c) 
{
	while(*c) {
		uart_putc(*c);
		c++;
	}
}