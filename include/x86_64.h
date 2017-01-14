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
extern void gdt_flush(uint64_t gdtr);


struct tss_64 {
	uint32_t res_3;
	uint64_t rsp[3];
	uint64_t res_2;
	uint64_t ist[7];
	uint16_t res_1[3];
	uint16_t io_map_base;
} __attribute__((packed));


struct idt_descriptor {
	uint16_t offset_low;
	uint16_t segment_selector;
	uint16_t type;	
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
} __attribute__((packed));

/**
 * @brief Interrupt handler stack frame
 */
struct registers
{
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rbx;
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t int_no;
	uint64_t err_no;
	uint64_t rip;
	uint64_t cs;
	uint64_t flags;
	uint64_t rsp;
	uint64_t ss;	
} __attribute__((packed));


uint64_t gdt[10] __attribute__((aligned(32)));

/* GDT Type flags, Bits 40:43 */
#define GDT_TSS_AVAIL 	0x9
#define GDT_TSS_BUSY 	0xB
#define GDT_CALL_GATE 	0xC
#define GDT_INT_GATE 	0xE 
#define GDT_TRAP_GATE 	0xF 

/* Interrupt descriptor table */
struct idt_descriptor idt[256];

/* Task state segment */
struct tss_64 system_tss;

/* Array of interrupt handlers */
extern uint64_t vectors[256];



#endif