#ifndef __MMU__
#define __MMU__

#include <stddef.h>

#define PRESENT (1<<0)
#define RW 		(1<<1)
#define USER 	(1<<2)
#define PWT		(1<<3)
#define PCD 	(1<<4)
#define ACCESS  (1<<5)
#define DIRTY 	(1<<6)
#define PS 		(1<<7)

#define PML4E(x) 	(((size_t) (x) >> 39) & 0x1FF)
#define PDPTE(x) 	(((size_t) (x) >> 30) & 0x1FF)
#define PDE(x) 		(((size_t) (x) >> 21) & 0x1FF)
#define PTE(x) 		(((size_t) (x) >> 12) & 0x1FF)

extern void mmu_init(void);
extern void mmu_bootstrap(size_t physical, size_t* pml4, size_t* pdpt, size_t* pd);
extern void mmu_map2mb(size_t physical, size_t address, int flags);
extern void mmu_map(uint64_t address);
extern struct page* mmu_req_page(uint64_t address, int flags);
void mmu_map_page(struct page* frame, size_t virtual, int flags);
size_t get_cr3(void);

#endif