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

/*
I/O APIC manages system-wide IO interrupt events on a SMP system
http://www.intel.com/design/chipsets/datashts/29056601.pdf
*/

#include <stdint.h>
#include <frame.h>
#include <arch/x86_64/kernel.h>
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/mmu.h>
#include <arch/x86_64/interrupts.h>

#define IOREGSEL	0x00 		/* This register selects the IOAPIC register to be read/written */
#define IOWIN		0x10 		/* This register is used to read and write from the reg selected by IOREGSEL */


#define IOAPICID 	0x00 		/* Identification register - 4 bit APIC ID */
#define IOAPICVER	0x01 
#define IOREDTBL 	0x10 		/* Base address of IO Redirection tables */


static size_t IOAPIC = 0;

static uint32_t ioapic_read(int reg)
{
	*(uint32_t*)(IOAPIC + IOREGSEL) = reg;
	return *(uint32_t*)(IOAPIC + IOWIN);
}

static void ioapic_write(int reg, uint32_t data)
{
	*(uint32_t*)(IOAPIC + IOREGSEL) = reg;
	*(uint32_t*)(IOAPIC + IOWIN) = data;
}

int ioapic_enable(uint8_t irq, uint16_t cpu)
{
	if (! IOAPIC)
		return -1;
	/* Write the low 32 bits :
	31:17 reserved
	16: Interrupt mask:		1 = masked. 0 = enabled
	15: Trigger mode:		1 = Level sensitive. 0 = Edge sensitive
	14: Remote IRR:			1 = LAPIC accept. 0 = LAPIC sent EOI, and IOAPIC received
	13: INTPOL: 			1 = Low active polarity. 0 = High active polarity
	12: Delivery Stat:		1 = Send Pending. 0 = IDLE
	11: Destination Mode:	1 = Logical Mode (Set of processors.. LAPIC id?). 0 = Physical mode, APIC ID
	10:8 Delivery Mode:
		000 Fixed
		001 Lowest priority
		010 SMI: System Management Interrupt. Requires edge trigger mode
		011 Reserved
		100 NMI
		101 INIT
		110 Reserved
		111 ExtINT
	7:0 Interrupt Vector: 8 bit field containing the interrupt vector, from 0x10 to 0xFE
	*/
	uint32_t low = (IRQ_ZERO + irq);
	kernel_log("[ioapic] Enabling irq %d on cpu %d (low dword: %#x)\n", irq, cpu, low);
	ioapic_write(IOREDTBL + (irq * 2), low );
	/* Write the high 32 bites. 63:56 contains destination field */
	ioapic_write(IOREDTBL + (irq * 2) + 1, cpu << 24);
	return 0;
}

int ioapic_remap(uint8_t src, uint8_t gsi, uint16_t cpu)
{
		if (! IOAPIC)
		return -1;
	/* Write the low 32 bits :
	31:17 reserved
	16: Interrupt mask:		1 = masked. 0 = enabled
	15: Trigger mode:		1 = Level sensitive. 0 = Edge sensitive
	14: Remote IRR:			1 = LAPIC accept. 0 = LAPIC sent EOI, and IOAPIC received
	13: INTPOL: 			1 = Low active polarity. 0 = High active polarity
	12: Delivery Stat:		1 = Send Pending. 0 = IDLE
	11: Destination Mode:	1 = Logical Mode (Set of processors.. LAPIC id?). 0 = Physical mode, APIC ID
	10:8 Delivery Mode:
		000 Fixed
		001 Lowest priority
		010 SMI: System Management Interrupt. Requires edge trigger mode
		011 Reserved
		100 NMI
		101 INIT
		110 Reserved
		111 ExtINT
	7:0 Interrupt Vector: 8 bit field containing the interrupt vector, from 0x10 to 0xFE
	*/
	uint32_t low = (IRQ_ZERO + gsi);
	kernel_log("[ioapic] Mapping %d to %d on cpu %d (low dword: %#x)\n", src, gsi, cpu, low);
	ioapic_write(IOREDTBL + (src * 2), low );
	/* Write the high 32 bites. 63:56 contains destination field */
	ioapic_write(IOREDTBL + (src * 2) + 1, cpu << 24);
	return 0;
}

void ioapic_disable(uint8_t irq)
{
	ioapic_write(IOREDTBL + (irq*2), (1<<16) | (0x20 + irq));
	ioapic_write(IOREDTBL + (irq*2) + 1, 0);
}

void ioapic_init(size_t ioapic_address)
{
	struct page ioapic_page;
	ioapic_page.address = ROUND_DOWN(ioapic_address, PAGE_SIZE);
	ioapic_address = P2V(ioapic_address);
	mmu_map_page(&ioapic_page, ioapic_address, PRESENT | RW);
	IOAPIC = ioapic_address;
	
	uint32_t ver = ioapic_read(0x01);
	uint16_t max = (ver >> 16) & 0xFF;

	kernel_log("[ioapic] version %#x max reg table entries: %d\n", ver, max);
	for (int i = 0; i < max; i++)
		ioapic_disable(i);
}



