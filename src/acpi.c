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

#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/kernel.h>
#include <arch/x86_64/mmu.h>
#include <arch/x86_64/interrupts.h>
#include <stdio.h>
#include <assert.h>
#include <acpi.h>
#include <frame.h>

static int acpi_checksum(char* ptr)
{
	int sum = 0;
	for (int i = 0; i < 20; ++i)
		sum += ptr[i];
	return (char) sum;

}

static int acpi_parse_madt(struct madt_header* madt)
{
	char* entries = (char*) ((size_t) madt + sizeof(struct madt_header));
	int num_lapics = 0;
	while( (size_t) entries < (madt->h.length - sizeof(struct madt_header)) + (size_t)madt) {		
		int type = *entries;
		int len = *(entries+1);
		switch(type) {
			case 0: {
				struct acpi_lapic *x = (struct acpi_lapic*) entries;
				kernel_log("[acpi] lapic: processor id %d\tapic id %d\tflags:%d\n", x->acpi_proc_id, x->apic_id, x->flags);
				num_lapics++;
				break;
			}
			case 1: {
				struct acpi_ioapic *x = (struct acpi_ioapic*) entries;
				kernel_log("[acpi] ioapic: id %d\taddress:0x%x\n", x->ioapic_id, x->ioapic_addr);
				ioapic_init(x->ioapic_addr);
				break;
			}
			case 2: {
				struct acpi_iso *x = (struct acpi_iso*)  entries;
				kernel_log("[acpi] irq src %d bus src %d gsi %d\n", x->irq_src, x->bus_src, x->gsi);
				ioapic_remap(x->irq_src, x->gsi, 0);
				break;
			}
			case 9: {
				struct acpi_x2apic* x = (struct acpi_x2apic*) entries;
				kernel_log("[acpi] x2apic: processor id %d\tflags: %d\n", x->apic_id, x->flags);
				num_lapics++;
				break;
			}
			default:
				kernel_log("[acpi] unsupported field: %d\n", type);
		}
		entries += len;
	} 
	return num_lapics;
} 

int acpi_init()
{
	/* Read through the extended bios data area (EBDA) and look at every 16-byte aligned
	 * structure for the signature */
    uint8_t *ptr = (uint8_t *) P2V(0x000E0000);
    int cpu_count = 0;

	while ((size_t) ptr < P2V(0x000FFFFF)) {
		uint64_t signature = *(uint64_t *)ptr;
		if (signature == 0x2052545020445352) { // 'RSD PTR '
			break;
		}
		ptr += 16;
	}

	struct rsdp_header* h = (struct rsdp_header*) ptr;

	/* Make sure we have a valid ACPI table. lower byte of the sum of the first 20 bytes
	 * (isn't that a mouthful?) need to sum to zero */
	assert(acpi_checksum((char*) h) == 0);
	kernel_log("[acpi] version %d table detected\n", h->rev);
	struct rsdt_header* r = (struct rsdt_header*) ((h->rev == 0) ? h->rsdt_ptr : h->xsdt_ptr);


	/* Most likely, our memory map has not included this region as valid memory
	 * so trying to request the address from the physical memory manager will 
	 * cause an issue. Instead we just make a workaround */
	struct page temp;
	temp.address = ROUND_DOWN(h->rsdt_ptr, PAGE_SIZE);
	mmu_map_page(&temp, ROUND_DOWN(P2V(h->rsdt_ptr), PAGE_SIZE), PRESENT|RW);
	assert((size_t) temp.data == ROUND_DOWN(P2V(h->rsdt_ptr), PAGE_SIZE));


	r = (struct rsdt_header*) P2V(r);
	for (int i = 0; i < (r->h.length - sizeof(struct acpi_header)) /4; i++) {
		struct acpi_header* entry = (struct acpi_header*) P2V(r->tableptrs[i]);
		temp.address = ROUND_DOWN(r->tableptrs[i], PAGE_SIZE);
		mmu_map_page(&temp, ROUND_DOWN(P2V(r->tableptrs[i]), PAGE_SIZE), PRESENT|RW);
		assert((size_t) temp.data == ROUND_DOWN(P2V(r->tableptrs[i]), PAGE_SIZE));

		char name[5];
		memcpy(name, entry->signature, 4);
		name[4] = '\0';
		kernel_log("[acpi] entry %s found\n", name);
		if (!strncmp(entry->signature, "APIC", 4)) {
			cpu_count = acpi_parse_madt((struct madt_header*) entry);			//return acpi_parse_madt((struct madt_header*) entry);
		}
	}
	return cpu_count;

}