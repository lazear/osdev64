#ifndef __FRAME__
#define __FRAME__

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <list.h>

struct page {
	size_t address;
	size_t flags;
	void* data;				/* Implementation specific. Pointer to virtual map address, generally */
	atomic_int refs;
	struct list lru; 		/** least recently used page list */
	struct list pages; 		/** free/used/kernel/dma/etc page lists */
};

#define P_FREE 		0x01
#define P_USED 		0x02 
#define P_KERNEL	0x04 
#define P_USER 		0x08 
#define P_VIRT 		0x10 
#define P_PHYS 		0x18 	/** In use by physical memory manager */
#define P_IMMUTABLE 0x20 	/** Not freeable */

extern struct page* page_alloc(void);
extern void page_init(size_t memory, size_t table);
extern int page_mark_range(size_t start, size_t end, int flags);
#endif