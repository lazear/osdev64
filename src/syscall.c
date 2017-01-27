#include <interrupts.h>

#include <common.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <msr.h>
#include <stdio.h>

struct jmp_buf sys_exit_buf;

extern void sys_exit(struct jmp_buf*, uint64_t val);


void syscall_handler(struct syscall* r)
{
	printf("SYSCALL %d\n", r->rax);

	if (r->rax == 1)
		printf("sys_write: %s\n", r->rsi);

	if (r->rax == 60) {
		printf("sys_exit\n");
		longjmp(&sys_exit_buf, r->rip);

	}
}


/**
 * @brief Initialize usage of "syscall" and "sysret" instructions.
 */

extern void* syscall_entry();
void syscall_init(void)
{
	uint64_t cs = 0x800000000;
	
	/* thanks to Bochs being stricter than QEMU, this
	 * bug was squashed */
	uint64_t EFER = readmsr(IA32_EFER);
	writemsr(IA32_EFER, EFER | (1 << IA32_EFER_SCE));
	/* Write CS to bits 32:47 of IA32_STAR MSR */
	writemsr(IA32_STAR, cs);
	/* Write RFLAGS mask to IA32_FMASK */
	writemsr(IA32_FMASK, 0x200);
	/* Write entry RIP to IA32_LSTAR */
	writemsr(IA32_LSTAR, (uint64_t) syscall_entry);
}

