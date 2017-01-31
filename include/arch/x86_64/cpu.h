#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>
#include <stdio.h>

#define MAX_CPU_COUNT 64

struct cpu {
	size_t id;
	size_t stack;
	int ncli;
	uint64_t gdt[10] __attribute__((aligned(32)));
	struct cpu* cpu;		/* CPU local storage */
} cpus[MAX_CPU_COUNT];

/* This works by using GCC thread local storage tactics... the global variable
cpu is linked to GS:0x0000000, and since GS is different in each CPU's 
Global Descriptor Table, we can separate them, so each cpu can only access it's 
local storage through one variable */
extern struct cpu* cpu;

#endif