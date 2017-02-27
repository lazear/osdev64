#ifndef __INTERRUPTS__
#define __INTERRUPTS__

#include <arch/x86_64/desc.h>
#include <arch/x86_64/setjmp.h>

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

extern struct jmp_buf sys_exit_buf;

typedef int (*trap_handler) (struct registers* r);
extern void trap_init(void);
extern void trap(struct registers* r);
extern void trap_register(int num, int (*handler)(struct registers*));

extern void pic_enable(int irq);
extern void pic_disable(void);
extern void pic_init(void);

extern void syscall_init(void);
extern void syscall_handler(struct syscall*);


int ioapic_enable(uint8_t irq, uint16_t cpu);
int ioapic_remap(uint8_t src, uint8_t gsi, uint16_t cpu);
void ioapic_disable(uint8_t irq);
void ioapic_init(uint64_t ioapic_address);

void lapic_init();
void lapic_test(uint16_t dest, uint16_t sh, uint16_t vector) ;


static inline void interrupts_disable(void)
{
	asm volatile("cli");
}

static inline void interrupts_enable(void)
{
	asm volatile("sti");
}

#endif