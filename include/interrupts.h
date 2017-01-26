#ifndef __INTERRUPTS__
#define __INTERRUPTS__

#include <desc.h>
#include <setjmp.h>

#define SYSCALL			0x80 
#define BREAKPOINT 		3 
#define PAGEFAULT 		14
#define IRQ_ZERO		0x20
#define IRQ_TIMER		0x00
#define IRQ_KBD			0x01
#define IRQ_COM1		0x04
#define IRQ_IDE			0x0D
#define IRQ_ERROR		0x13
#define IRQ_SPURIOUS	0xDF

struct syscall {
	uint64_t rax;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rip;
	uint64_t rflags;
	uint64_t rbx;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rsp;
};

extern struct jmp_buf sys_exit_buf;


typedef void (*trap_handler) (struct registers* r);
extern void trap_init(void);
extern void trap(struct registers* r);
extern void trap_register(int num, void (*handler)(struct registers*));

extern void pic_enable(int irq);
extern void pic_disable(void);
extern void pic_init(void);

extern void syscall_init(void);
extern void syscall_handler(struct syscall*);

static inline void cli(void)
{
	asm volatile("cli");
}

static inline void sti(void)
{
	asm volatile("sti");
}

#endif