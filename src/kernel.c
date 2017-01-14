#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "x86_64.h"


extern int printf(const char* fmt, ...);
extern void gdt_init(void);
extern void idt_init(void);

#define PIC1_CMD		0x20
#define PIC1_DATA		0x21
#define PIC2_CMD		0xA0
#define PIC2_DATA		0xA1

#define IRQ_SLAVE       2       /* IRQ at which slave connects to master */

/* Initial IRQ mask has interrupt 2 enabled (for slave 8259A).*/
static uint16_t irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);

extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

/* Mask interrupts on the PIC */
static void pic_setmask(uint16_t mask) 
{
	irqmask = mask;
	outb(PIC1_DATA, mask);
	outb(PIC2_DATA, mask >> 8);
}

void pic_enable(int irq) 
{
 	pic_setmask(irqmask & ~(1<<irq));
}

/* Mask all interrupts */
void pic_disable() 
{
	pic_setmask(0xFFFF);
}

/* Initialize the 8259A interrupt controllers. */
void pic_init(void) 
{
	/* mask all interrupts */
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	/* Set up master (8259A-1) */
	outb(PIC1_CMD, 0x11);

	/* ICW2:  Vector offset */
	outb(PIC1_DATA, 32);
	outb(PIC1_DATA, 1<<IRQ_SLAVE);

	outb(PIC1_DATA, 0x3);

	/* Set up slave (8259A-2) */
	outb(PIC2_CMD, 0x11);
	outb(PIC2_DATA, 32 + 8); 
	outb(PIC2_DATA, IRQ_SLAVE);
	outb(PIC2_DATA, 0x3);

	outb(PIC1_CMD, 0x68);
	outb(PIC1_CMD, 0x0a);

	outb(PIC2_CMD, 0x68);
	outb(PIC2_CMD, 0x0a);

	if(irqmask != 0xFFFF)
		pic_setmask(irqmask);
}

struct memory_map
{
	uint64_t base;
	uint64_t len;
	uint64_t type;
};

static char* exceptions[] = {
	"Divide by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor",
	"Invalid TSS",
	"Segment not present",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 FPU",
	"Alignment check",
	"Machine check",
	"SIMD FP",
	"Virtualization fault",
	"Reserved",
	[0x1E] = "Security fault",
};

extern void halt_catch_fire(void);
void trap(struct registers* r)
{
	if (r->int_no < 0x20) {
		asm volatile("cli");
		/* safe printing... */
		char buf[100];
		snprintf(buf, 100, "CPU exception: %s\n", exceptions[r->int_no]);
		vga_puts(buf);

		halt_catch_fire();
	}

	outb(0x20, 0x20);
}

char buf[100];
void main(void)
{
	gdt_init();
	idt_init();
	pic_init();

	vga_clear();

	uint8_t memlen = *(uint8_t*) 0x6FF0 / sizeof(struct memory_map);
	struct memory_map* mmap = (struct memory_map*) 0x7000;
	struct memory_map* mmax = mmap + memlen;
	printf("Control transferred to x86_64 kernel\n");


	uint64_t phys_mem_detected = 0;
	while (mmap < mmax) {
		printf("%8s memory detected %#x - %#x\n", (mmap->type & 0xF) == 1 ? "Usable" : "Reserved", mmap->base, mmap->base + mmap->len);
		if ((mmap->type & 0xF) == 1)
			phys_mem_detected += mmap->len;
		mmap++;
	}

	printf("%dM total physical memory detected\n", phys_mem_detected / 0x100000);


	asm volatile("sti");

	pic_enable(32);
	for(;;);
}
