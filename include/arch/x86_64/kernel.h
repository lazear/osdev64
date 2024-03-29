#ifndef __X86_64_COMMON__
#define __X86_64_COMMON__

#include <stddef.h>
#include <stdint.h>


#define KERNEL_CS 0x08

extern void memset(void*, int c, size_t n);
extern void memsetw(void*, int c, size_t n);
extern void memcpy(void*, const void*, size_t n);
extern size_t strlen(char* s);
extern char* strncpy(char* dest, const char* src, size_t n);
extern char* strcpy(char* dest, const char* src);
extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint16_t ind(uint16_t port);
extern void outb(uint16_t port, uint8_t data);
extern void outw(uint16_t port, uint16_t data);
extern void outd(uint16_t port, uint32_t data);
extern void writemsr(size_t reg, size_t data);
extern size_t readmsr(size_t reg);
int strncmp(const char *s1, const char *s2, size_t n);

extern void halt_catch_fire(void);

//extern size_t KERNEL_VIRT;
//extern size_t KERNEL_PHYS;

#define KERNEL_VIRT		0xFFFFFFFF80000000
#define INITIAL_TOP		0xFFFFFFFF80400000

#define PAGE_SIZE 0x1000

#define ROUND_DOWN(a, b) ((a) & ~((b) - 1))

#define ROUND_UP(a, b) (((a) + (b)) & ~((b) - 1))

#define likely(x) 	__builtin_expect (x, 1)
#define unlikely(x) __builtin_expect (x, 0)

#ifndef NULL
#define NULL ((void*) 0)
#endif

// Convert physical to higher half.
#define P2V(x)	((size_t) (x) | KERNEL_VIRT) //( ((size_t) (x) <= KERNEL_VIRT) ? ((size_t) (x) + KERNEL_VIRT) : (size_t) (x))
#define V2P(x)	((size_t) (x) & ~KERNEL_VIRT)

extern size_t _kernel_start;
extern size_t _kernel_end;

#define KERNEL_START ((size_t)&_kernel_start)
#define KERNEL_END ((size_t)&_kernel_end)

#endif