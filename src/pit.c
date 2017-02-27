#include <arch/x86_64/kernel.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/timer.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static volatile uint64_t timer_ticks = 0;

int pit_handler(struct registers* r)
{
	if (r->cs != KERNEL_CS) {
		/* potential schedule? */
		kernel_log("[timer] PIT still running with usermode...\n");
	}
	timer_ticks++;
	return 0;
}

uint64_t pit_get_ticks(void)
{
	return timer_ticks;
}

void pit_init(uint32_t frequency)
{
	int divisor = 1193180 / frequency;
	trap_register(IRQ_TIMER + IRQ_ZERO, pit_handler);
	outb(0x43, 0x34);
	inb(0x43);
	outb(0x40, divisor & 0xFF);
	inb(0x40);
	outb(0x40, divisor >> 8);
	kernel_log("[timer] Initializing PIT with frequency %d hz\n", frequency);
}

void pit_shutdown()
{
	interrupts_disable();
	trap_register(IRQ_TIMER + IRQ_ZERO, NULL);
	outb(0x42, 0x34);
	interrupts_enable();
}