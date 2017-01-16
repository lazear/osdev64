
#include <common.h>
#include <stdint.h>
#include <stddef.h>
#include <msr.h>

void syscall(void)
{
	printf("SYSCALL");
	for(;;);	
}

void syscall_config(void)
{
	writemsr(IA32_EFER, 1);
	writemsr(IA32_STAR, (uint64_t) (0x8 << 32));
	writemsr(IA32_FMASK, 0x202);
	writemsr(IA32_LSTAR, (uint64_t) syscall);
}

