#ifndef __X86_64_COMMON__
#define __X86_64_COMMON__

#include <stddef.h>
#include <stdint.h>

extern void memset(void*, int c, size_t n);
extern void memsetw(void*, int c, size_t n);
extern void memcpy(void*, void*, size_t n);
extern int strlen(char* s);
extern char* strncpy(char* dest, const char* src, size_t n);
extern char* strcpy(char* dest, const char* src);
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t data);
extern void writemsr(size_t reg, size_t data);
extern size_t readmsr(size_t reg);



#endif