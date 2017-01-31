#ifndef __KSTDIO__
#define __KSTDIO__

#include <stdarg.h>

extern int kernel_log(const char* fmt, ...);
extern int printf(const char* fmt, ...);
extern int snprintf(char* str, size_t n, const char* fmt, ...);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap) ;
extern void vga_init();
extern void vga_clear();
extern void vga_puts(char* s);
extern void vga_putc(char);
extern void vga_kputc(char c, int x, int y);
extern void vga_putc(char c);
extern void vga_setcolor(int color);
extern void vga_scroll();
extern void vga_pretty(char* s, int color);
extern int vga_current_x();
extern int vga_current_y();

#define VGA_BLACK	0x00
#define VGA_BLUE	0x01
#define VGA_GREEN	0x02
#define VGA_CYAN	0x03
#define VGA_RED		0x04
#define VGA_MAGENTA	0x05
#define VGA_BROWN	0x06
#define VGA_LIGHTGREY	0x07
#define VGA_DARKGREY	0x08
#define VGA_LIGHTBLUE	0x09
#define VGA_LIGHTGREEN	0x0A
#define VGA_LIGHTCYAN	0x0B
#define VGA_LIGHTRED	0x0C
#define VGA_LIGHTMAGENTA	0x0D
#define VGA_LIGHTBROWN		0x0E
#define VGA_WHITE		0x0F
#define VGA_COLOR(f, b)	((b << 4) | (f & 0xF))

#endif