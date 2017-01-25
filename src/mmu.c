
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include <assert.h>
#include <stdio.h>
#include <list.h>
#include <common.h>
#include <frame.h>
#include <mmu.h>


static struct page* PML4;

void mmu_init(void)
{
	size_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3));

	PML4 = page_request(cr3);
	assert(PML4);
	PML4->data = (void*) cr3;
	PML4->flags = P_USED | P_IMMUTABLE | P_KERNEL | P_VIRT;
	list_del(&PML4->pages);
	list_del(&PML4->lru);
}


/**
 * @brief Map enough virtual memory to contain our physical page array.
 * @details 2 megabyte pages are identity mapped, and mapped to KERNEL_VIRT, up
 * to physical address provided. PML4 and associated structures will be reloaded 
 * at the end of the call. All parameters provided should be previously mapped
 * and accessible without page faulting. This provides enough overhead to map up 
 * to 4 GB of physical pages.
 * 
 * @param physical Amount of memory needed for array.
 * @param pml4 Physical page address of new PML4.
 * @param pdpt Physical page address of new PDPT.
 * @param pd Physical page address of new PD.
 */
void mmu_bootstrap(size_t physical, size_t* pml4, size_t* pdpt, size_t* pd)
{

	dprintf("[mmu ] bootstrap %#x\n", physical);	
	int i;

	pml4[PML4E(0)] 				= ((size_t) pdpt) | (PRESENT | RW);
	pml4[PML4E(KERNEL_VIRT)] 	= ((size_t) pdpt) | (PRESENT | RW);
	pdpt[PDPTE(0)] 				= ((size_t) pd) | (PRESENT | RW);
	for (i = 0; i < physical; i += 0x00200000) {
		pd[PDE(i)] = i | (PRESENT | RW | PS);
	}

	//asm volatile("cli");
	asm volatile("mov %0, %%cr3" : : "r"(pml4));
}


void mmu_map2mb(size_t physical, size_t address, int flags) 
{
	size_t* pml4 = PML4->data;
	size_t* pdpt = NULL;
	size_t* pd = NULL;

	physical = ROUND_DOWN(physical, (1<<21));
	printf("physical address %x", physical);

	if (pml4[PML4E(address)] & PRESENT) {
		pdpt = (size_t*) ROUND_DOWN(pml4[PML4E(address)], PAGE_SIZE);
	} else {
		struct page* p = page_alloc();
		pdpt = (size_t*) p->address;
		pml4[PML4E(address)] = ((size_t) pd) | (PRESENT | RW);
	}

	/* Check for existing page directory */
	if (pdpt[PDPTE(address)] & PRESENT) {
		if (pdpt[PDPTE(address)] & PS) {
			/* Map a 1GB page */
		}
		pd = (size_t*) ROUND_DOWN(pdpt[PDPTE(address)], PAGE_SIZE);
	} else {
		struct page* p = page_alloc();
		pd = (size_t*) p->address;
		pdpt[PDPTE(address)] = ((size_t) pd) | (PRESENT | RW);
	}

	pd[PDE(address)] = (physical | flags) | PS;
}

void mmu_map(uint64_t address)
{
	size_t* pml4 = PML4->data;
	size_t* pdpt = NULL;
	size_t* pd = NULL;
	size_t* pt = NULL;
	size_t phys;

	if (pml4[PML4E(address)] & PRESENT) {
		pdpt = (size_t*) ROUND_DOWN(pml4[PML4E(address)], PAGE_SIZE);
	} else {
		struct page* p = page_alloc();
		pdpt = (size_t*) p->address;
		pml4[PML4E(address)] = ((size_t) pd) | (PRESENT | RW);
	}

	/* Check for existing page directory */
	if (pdpt[PDPTE(address)] & PRESENT) {
		if (pdpt[PDPTE(address)] & PS) {
			/* Map a 1GB page */
		}
		pd = (size_t*) ROUND_DOWN(pdpt[PDPTE(address)], PAGE_SIZE);
	} else {
		struct page* p = page_alloc();
		pd = (size_t*) p->address;
		pdpt[PDPTE(address)] = ((size_t) pd) | (PRESENT | RW);
	}

	if (pd[PDE(address)] & PRESENT) {
		if (pd[PDE(address)] & PS) {
			/* Map a 2MB page */
		}
		pt = (size_t*) ROUND_DOWN(pd[PDE(address)], PAGE_SIZE);
	} else {
		struct page* p = page_alloc();
		pt = (size_t*) p->address;
		pd[PDE(address)] = ((size_t) pd) | (PRESENT | RW);
	}
	

	phys = ROUND_DOWN(pt[(address >> 12) & 0x1FF], PAGE_SIZE);

	printf("pml4 %x\n", pml4);
	printf("pdpt %x\n", pdpt);
	printf("pd   %x\n", pd);
	printf("pt   %x\n", pt);
	printf("phys %x\n", phys);
	// if (((size_t) pml4) & 0xFFF == 0)
	// 	printf("ERROR!\n");


	// uint64_t pdp, pd, pt, offset;
	// pdp		= (address >> 30) & 0x1FF;
	// pd 		= (address >> 21) & 0x1FF;
	// pt 		= (address >> 12) & 0x1FF;
	// offset  = (address) & 0xFFF;
	

	// printf("cr3:\t%#x\n", PML4->data);
	// printf("pml4:\t%#x\n", pml4);
	// printf("pdp:\t%#x\n",  pdp);

	// printf("pd:\t%#x\n",pd);
	// printf("pt:\t%#x\n", pt);
	// printf("offset:\t%#x\n", offset);
}