/*
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

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
===============================================================================
*/



#ifndef __LAPIC__
#define __LAPIC__

#define LAPIC_BASE 		0xFEE00000		/* Physical Address of MMIO. 4kb */
/* LAPIC Memory Mapped Registers */
#define LAPIC_ID		0x020
#define LAPIC_VER		0x030
#define LAPIC_TPR		0x080	// Task Priority Register R/W
#define LAPIC_APR		0x090 	// Arbitration Priority Register
#define LAPIC_PPR 		0x0A0	// Processor Priority Register
#define LAPIC_EOI		0x0B0	// EOI - Write Only
#define LAPIC_SIV		0x0F0 	// Spurious Int. Vector
#define LAPIC_ERR 		0x280	// Error Status Register
#define LAPIC_ICRLO		0x300 	// Interrupt Command Register
#define LAPIC_ICRHI		0x310 	// Interrupt Command Register

#define TIMER 			0x320 	// LVT Timer Register
#define INIT_COUNT 		0x380	// Timer Init. Count Register
#define CURR_COUNT 		0x390 	// Timer Current Count Register
#define DIVIDE_CONF		0x3E0	// Timer Divide Config. Register

#define LAPIC_LINT0		0x350 	// Local Int 
#define LAPIC_LINT1		0x360 	// Local Int
#define LAPIC_LINTERR	0x370 	// LVT Errors


#define INIT       0x00000500   // INIT/RESET
#define STARTUP    0x00000600   // Startup IPI
#define DELIVS     0x00001000   // Delivery status
#define ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define DEASSERT   0x00000000
#define LEVEL      0x00008000   // Level triggered
#define BCAST      0x00080000   // Send to all APICs, including self.
#define BUSY       0x00001000
#define FIXED      0x00000000


/* TImer definitions */
#define ONESHOT		0
#define PERIODIC	1
#define TSCDEADLINE	2

extern int lapic_active();
extern int lapic_danger_id();
extern void lapic_eoi();
extern void lapic_test(uint16_t, uint16_t, uint16_t);
extern void lapic_timer_config(uint8_t mode, uint32_t initial_count, uint8_t divide_by);
extern void lapic_init();
extern int mp_processor_id();
extern int mp_number_of_processors();
extern void mp_start_ap(int nproc);

#endif