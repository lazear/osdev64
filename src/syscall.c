/*
MIT License
Copyright (c) 2016-2017 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <arch/x86_64/interrupts.h>

#include <arch/x86_64/kernel.h>
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/setjmp.h>
#include <arch/x86_64/msr.h>
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

