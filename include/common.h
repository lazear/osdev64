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

extern void halt_catch_fire(void);

#define KERNEL_VIRT		0xFFFF800000000000
#define INITIAL_TOP		0xFFFF800000400000

#define PAGE_SIZE 0x1000

#define ROUND_DOWN(a, b) ((a) & ~((b) - 1))

#define ROUND_UP(a, b) (((a) + (b)) & ~((b) - 1))

#define likely(x) 	__builtin_expect (x, 1)
#define unlikely(x) __builtin_expect (x, 0)

#ifndef NULL
#define NULL ((void*) 0)
#endif

// Convert physical to higher half.
#define P2V(x)	( ((size_t) (x) <= KERNEL_VIRT) ? ((size_t) (x) + KERNEL_VIRT) : (size_t) (x))
#define V2P(x)	( ((size_t) (x) >= KERNEL_VIRT) ? ((size_t) (x) - KERNEL_VIRT) : (size_t) (x))

extern size_t _kernel_start;
extern size_t _kernel_end;

#define KERNEL_START ((size_t)&_kernel_start)
#define KERNEL_END ((size_t)&_kernel_end)

#endif