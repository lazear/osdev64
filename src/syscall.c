#include <interrupts.h>
#include <common.h>
#include <stdint.h>
#include <stddef.h>
#include <msr.h>

void syscall(struct registers* r)
{
	printf("SYSCALL\n");
	for(;;);	
}


/**
 * @brief Initialize usage of "syscall" and "sysret" instructions.
 */
void syscall_init(void)
{
	uint64_t cs = 0x800000000;
	/* Enable "syscall" instruction */
	writemsr(IA32_EFER, 1 << IA32_EFER_SCE);
	/* Write CS to bits 32:47 of IA32_STAR MSR */
	writemsr(IA32_STAR, cs);
	/* Write RFLAGS mask to IA32_FMASK */
	writemsr(IA32_FMASK, 0x200);
	/* Write entry RIP to IA32_LSTAR */
	writemsr(IA32_LSTAR, (uint64_t) syscall);
}

