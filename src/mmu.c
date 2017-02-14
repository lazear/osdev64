
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include <assert.h>
#include <stdio.h>
#include <list.h>
#include <arch/x86_64/kernel.h>
#include <frame.h>
#include <arch/x86_64/mmu.h>

static struct page* PML4;

void* __to_address(uint64_t pml4, uint64_t pdp, uint64_t pd, uint64_t pt)
{
	uint64_t ret = (pml4 << 39);
	if (ret & (1LL << 47)) {
		/* Bits 48:64 must mirror bit 47 */
		ret |= 0xFFFF000000000000;
	}
	ret |= (pdp << 30);
	ret |= (pd  << 21);
	ret |= (pt  << 12);
	return (void*)ret;
}

size_t get_cr3(void)
{
	size_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

void mmu_init(void)
{
	/* mmu_bootstrap MAY have been called before mmu_init, so reload cr3
	 * just in case */
	size_t cr3 = get_cr3();

	PML4 = page_request(cr3);
	assert(PML4);
	PML4->data = (void*) __to_address(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
	PML4->flags = P_USED | P_IMMUTABLE | P_KERNEL | P_VIRT;
	list_del(&PML4->pages);
	list_del(&PML4->lru);
	size_t* pml4 = (size_t*) P2V(cr3);

	pml4[RECURSIVE_ENTRY] = (size_t) cr3 | (PRESENT | RW);
}


/**
 * @brief Convert a virtual address to physical address.
 * @details Use recursive page mapping to access paging structures.
 * NOTE: This only works for 4KB pages, and we need to be careful since we
 * mix 2mb and 4kb pages.
 * 
 * @return Physical address
 */
size_t mmu_virt_to_phys(size_t virt)
{
	/*
	size_t* pdpt = __to_address(RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4E(virt));
	size_t* pd 	 = __to_address(RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4E(virt), PDPTE(virt));
	*/
	size_t* pt   = __to_address(RECURSIVE_ENTRY, PML4E(virt), PDPTE(virt), PDE(virt));
	return pt[PTE(virt)];
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

	kernel_log("[mmu ] bootstrap %#x, pml4 %#x\n", physical, pml4);	
	int i;

	/* Map in address, and recursively map in PML4 */
	pml4[PML4E(KERNEL_VIRT)] 	= ((size_t) pdpt) | (PRESENT | RW);
	pml4[RECURSIVE_ENTRY] = ((size_t) pml4) | (PRESENT | RW);
	pdpt[PDPTE(physical)] = ((size_t) pd) | (PRESENT | RW);
	for (i = 0; i < physical; i += 0x00200000) {
		pd[PDE(i)] = i | (PRESENT | RW | PS);
	}

	asm volatile("mov %0, %%cr3" : : "r"(pml4));
}



void mmu_map2mb(size_t physical, size_t address, int flags) 
{
	size_t* pml4 = PML4->data;
	size_t* pdpt = NULL;
	size_t* pd = NULL;

	physical = ROUND_DOWN(physical, (1<<21));

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


void mmu_map_page(struct page* frame, size_t address, int flags)
{
	size_t* pml4 = PML4->data;
	size_t* pdpt = NULL;
	size_t* pd = NULL;
	size_t* pt = NULL;

	assert(frame);
	assert(pml4);
	kernel_log("[mmu ] mapping requested: phys %#x -> virt %#x (%x)\n", frame->address, address, flags);

	if (pml4[PML4E(address)] & PRESENT) {
		pdpt = (size_t*) P2V(ROUND_DOWN(pml4[PML4E(address)], PAGE_SIZE));
	} else {
		struct page* p = page_alloc();
		assert(p);
		kernel_log("[mmu ] allocating page %#x for new PDPT\n", p->address);
		pdpt = (size_t*) P2V(p->address);
		pml4[PML4E(address)] = ((size_t) p->address) | (PRESENT | RW);
	}

	/* Check for existing page directory */
	assert(pdpt);
	if (pdpt[PDPTE(address)] & PRESENT) {
		if (pdpt[PDPTE(address)] & PS) {
			/* Map a 1GB page */
		}
		pd = (size_t*) P2V(ROUND_DOWN(pdpt[PDPTE(address)], PAGE_SIZE));
	} else {
		struct page* p = page_alloc();
		assert(p);
		kernel_log("[mmu ] allocating page %#x for new PD\n", p->address);
		pd = (size_t*) P2V(p->address);
		pdpt[PDPTE(address)] = ((size_t) p->address) | (PRESENT | RW);
		//pdpt[RECURSIVE_ENTRY] = ((size_t) pdpt) | (PRESENT | RW);
	}
	assert(pd);
	if (pd[PDE(address)] & PRESENT) {
		if (pd[PDE(address)] & PS) {
			/* Map a 2MB page */
		}
		pt = (size_t*) P2V(ROUND_DOWN(pd[PDE(address)], PAGE_SIZE));
	} else {
		struct page* p = page_alloc();
		assert(p);
		kernel_log("[mmu ] allocating page %#x for new PT\n", p->address);
		pt = (size_t*) P2V(p->address);
		pd[PDE(address)] = ((size_t) p->address) | (PRESENT | RW);
	}
	assert(pt);
	pt[PTE(address)] = (frame->address | flags);
	frame->data = (void*) address;
}

struct page* mmu_req_page(uint64_t address, int flags)
{
	struct page* page = page_alloc();
	address = ROUND_DOWN(address, PAGE_SIZE);
	mmu_map_page(page, address, flags);
	if ((size_t) page->data != address)
		return NULL;
	return page;
}