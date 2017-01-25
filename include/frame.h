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

#define P_FREE 		0x01000
#define P_USED 		0x02000
#define P_KERNEL	0x04000 
#define P_USER 		0x08000
#define P_VIRT 		0x10000
#define P_PHYS 		0x18000	/** In use by physical memory manager */
#define P_IMMUTABLE 0x20000	/** Not freeable */

extern struct page* page_request(size_t address);
extern struct page* page_alloc(void);
extern void page_free(struct page* p);

extern void page_init(size_t memory, size_t table);
extern int page_mark_range(size_t start, size_t end, int flags);

#endif