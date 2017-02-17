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

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <lock.h>
#include <arch/x86_64/kernel.h>
#include <stdio.h>

static struct vga_coord {
	uint8_t* vga;
	uint16_t attrib;
	uint16_t x, y;
	struct lock lock;
} __tty_info = {
	.vga = (uint8_t*) P2V(0xB8000),
	.attrib = 0x7,
	.x = 0,
	.y = 0,
};


void vga_clear() {
	uint8_t* vga_address = __tty_info.vga;

	const long size = 80 * 25;
	for (long i = 0; i < size; i++ ) { 
		*vga_address++ = 0; 				/* character value */
		*vga_address++ = __tty_info.attrib; 	/* color value */
	}
	__tty_info.y = 0;
	__tty_info.x = 0;
}

void vga_move_cursor(uint16_t x, uint16_t y) {
	uint16_t pos = (y*80) + x;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}


void vga_update_cursor() {
	vga_move_cursor(__tty_info.x/2, __tty_info.y);
}

void vga_setcolor(int color) {
	__tty_info.attrib = color;
}


/* Scroll the screen up one line */
void vga_scroll()
{
	if( __tty_info.y >= 25)
	{
		uint8_t* vga_addr = __tty_info.vga;
		uint8_t temp = __tty_info.y - 24;
		memcpy(vga_addr, vga_addr + temp * 160, (25 - temp) * 160 * 2);
		memsetw(vga_addr + (24*160), 0 | (__tty_info.attrib)<<8, 160);
		//__tty_info.x = 0;
		__tty_info.y = 24;
	}
	vga_update_cursor();
}

void vga_overwrite_color(int color, int start_x, int start_y, int end_x, int end_y) {
	int sizeend = 2*end_x + (160* end_y);
	for (int i = (start_x+1 + (160*start_y)); i < sizeend; i += 2) 
		__tty_info.vga[i] = color;
}

/* kernel level putc, does not update CURRENT_POSITION */
void vga_kputc(char c, int x, int y) {
	uint8_t *vga_address = __tty_info.vga + (x + y * 160);
	*(uint16_t*)vga_address = c | (__tty_info.attrib << 8);
}

void vga_kputs(char* s, int x, int y) {
	while (*s != 0)
		vga_kputc(*s++, x+=2, y);
}

void vga_puts(char* s) {
	lock_acquire(&__tty_info.lock);
	while (*s != 0) 
		vga_putc(*s++);
	lock_release(&__tty_info.lock);
}

/* Automatically update text position, used in vga_puts */
void vga_putc(char c) {
	if (c == '\n') {
		__tty_info.y += 1;
		__tty_info.x = 0;
		vga_scroll();
		return;
	} else if (c == '\b') {
		__tty_info.x -= 2;
		vga_kputc(' ', __tty_info.x, __tty_info.y);
		return;
	} else if (c == '\t') {
		while(__tty_info.x % 16) {
			__tty_info.x++;
		}
		__tty_info.x += 2;

		if (__tty_info.x >= 160) {
			__tty_info.x = 0;
			__tty_info.y += 1;
		}
		return;
	}

	vga_kputc(c, __tty_info.x, __tty_info.y);
	
	__tty_info.x += 2;
	if (__tty_info.x >= 160) {
		__tty_info.x = 0;
		__tty_info.y += 1;
	}
	vga_scroll();

}

/* vga_puts() with color */
void vga_pretty(char* s, int color) {
	int start_x = __tty_info.x;
	int start_y = __tty_info.y;
	vga_puts(s);
	vga_overwrite_color(color, start_x, start_y, 80, __tty_info.y);
}