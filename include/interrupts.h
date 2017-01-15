#ifndef __INTERRUPTS__
#define __INTERRUPTS__

#include <desc.h>

#define SYSCALL			0x80 
#define IRQ_ZERO		0x20
#define IRQ_TIMER		0x00
#define IRQ_KBD			0x01
#define IRQ_COM1		0x04
#define IRQ_IDE			0x0D
#define IRQ_ERROR		0x13
#define IRQ_SPURIOUS	0xDF

extern void trap(struct registers* r);

extern void pic_enable(int irq);
extern void pic_disable(void);
extern void pic_init(void);

static inline void cli(void)
{
	asm volatile("cli");
}

static inline void sti(void)
{
	asm volatile("sti");
}

#endif