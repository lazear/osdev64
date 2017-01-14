/*
printf.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

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
Kernel level printf. Should be removed once userspace is up and running
*/

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

#include "x86_64.h"

#define isdigit(c)		(c >= '0' && c <= '9')

#define PREFIX			1
#define ALWAYS_SIGN		2
#define PAD 			4
#define LONG			8
#define NOTNUM			0x10



int strrev(char* s)
{
	int length = strlen(s) - 1;
	int i;
	for (i = 0; i <= length/2; i++) {
		char temp = s[i];
		s[i] = s[length-i];
		s[length-i] = temp;
	}
	return length+1;
}

int ulltostr(size_t num, char* buffer, int base, int len) 
{
	int i = 0;
	if (num == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	while (num != 0 && len--) {
		size_t remainder = num % base;
		buffer[i++] = (remainder > 9)? (remainder - 10) + 'A' : remainder + '0';
		num = num / base;
	}
	buffer[i] = '\0';
	
	return strrev(buffer);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	char buf[size];
	memset(buf, 0, size);
	size_t n = 0;
	size_t pad = 0;
	int flags = 0;

	while(*format && n < size) {
		switch(*format) {
			case '%': {
				format++;

next_format:
				switch(*format) {
					case '%': {
						buf[0] = '%';
						buf[1] = '\0';
						break;
					}
					case '#': {
						flags |= PREFIX | PAD;
						pad = 16;
						format++;
						goto next_format;
					}
					case '+': {
						flags |= ALWAYS_SIGN;
						format++;
						goto next_format;
					}
					case 'x': {
						ulltostr(va_arg(ap, size_t), buf, 16, 16);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
							str[n++] = 'x';
						}
						break;
					}
					case 'd':{
						size_t d = va_arg(ap, size_t);
						if ((flags & ALWAYS_SIGN) && (n+1 < size)) 
							str[n++] = (d > 0) ? '+' : '-';
						ulltostr(d, buf, 10, 20);				
						break;
					}
				 	case 'i': {
						size_t d = va_arg(ap, size_t);
						if ((n+1 < size)) 
							if (d < 0) str[n++] = '-';
						ulltostr(d, buf, 10, 20);
				
						break;
					}
					case 'u': {
						ulltostr(va_arg(ap, size_t), buf, 10, 20);
						break;
					}
					case 'l': {
							flags |= LONG;
						ulltostr(va_arg(ap, size_t), buf, 16, 16);
						break;
					}
					case 'o': {
						ulltostr(va_arg(ap, size_t), buf, 8, 16);
						if ((flags & PREFIX) && (n+2 < size)) {
							str[n++] = '0';
						}
						break;
					}
					case 's': {
						char* tmp = va_arg(ap, char*);
						strncpy(buf, tmp, size - n);
						flags |= NOTNUM;
						break;
					}
					case 'c': {
						buf[0] = (char) va_arg(ap, int);
						buf[1] = '\0';
						flags |= NOTNUM;
						break;
					}
					case 'w': {
						flags |= PAD;
						buf[0] = ' ';
						break;
					}
					default: {
						/* Parse out padding information */
						pad = 0;
						while(isdigit(*format)) {
							flags |= PAD;
							pad *= 10;
							pad += *format - '0';
							format++;
						}
						goto next_format;
					}

				}
				if ((flags & PAD) && (pad > strlen(buf))) {
					for (int i = 0; (i < (pad - strlen(buf))) && ((i+n) < size); i++)
						str[n++] = (flags & NOTNUM) ? ' ' : '0';
				}
				for (int i = 0; (i < strlen(buf)) && ((i+n) < size); i++) 	
					str[n++] = buf[i];		
				memset(buf, 0, strlen(buf));
				break;
			}
			default: {
				str[n++] = *format;
				flags = 0;
				break;
			}
		}
		format++;
	}

	return n;
}

int snprintf(char* str, size_t n, const char* fmt, ...) 
{
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vsnprintf(str, n, fmt, ap);
	va_end(ap);
	return r;
}

static char buf[256];
int printf(const char* fmt, ...)
{
	int r;

	memset(buf, 0, 256);
	va_list ap;
	va_start(ap, fmt);
	r = vsnprintf(buf, 256, fmt, ap);
	va_end(ap);

	vga_puts(buf);

	return r;
}