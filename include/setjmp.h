#ifndef __SETJMP__
#define __SETJMP__

#include <stdint.h>

struct jmp_buf {
	uint64_t rbx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
};

extern uint64_t setjmp(struct jmp_buf*);
extern void longjmp(struct jmp_buf*, uint64_t value);

#endif