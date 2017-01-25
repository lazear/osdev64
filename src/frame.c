/* frame.c
===============================================================================
MIT License
Copyright (c) Michael Lazear 2016-2017 

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

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include <assert.h>
#include <stdio.h>
#include <list.h>
#include <lock.h>
#include <common.h>
#include <frame.h>

static struct lock page_lock;

static list(free);
static list(used);
static list(lru);

static size_t numpages;
static struct page* parray;

/**
 * @brief Initialize physical memory manager.
 * @details The physical memory manager using linked lists of struct pages to 
 * keep track of free and in-use pages. However, pages are also placed in an 
 * array by order of address, to allow O(1) lookup for specific addresses.
 * 
 * @param memory Highest allocatable physical address.
 * @param table Physical region for start of table.
 */
void page_init(size_t memory, size_t table)
{
	assert(memory > table);

	table = ROUND_UP(table, PAGE_SIZE);
	numpages = ROUND_UP(memory, PAGE_SIZE) / PAGE_SIZE;
	parray = (void*) table;

	int r = ROUND_UP((numpages * sizeof(struct page)), PAGE_SIZE) / PAGE_SIZE;
	int i = 0;

	if ((r*PAGE_SIZE + table) > V2P(INITIAL_TOP)) {
		int req = (r*PAGE_SIZE + table);
		printf("Remove RAM and try again\nNeed to map up to %x\n", req);
		halt_catch_fire();
	}

	/* Initialize all pages in system */
	for (i = 0; i < numpages; i++) {
		struct page* p = &parray[i];
		list_init(&p->lru);
		list_init(&p->pages);
		p->address = (i * PAGE_SIZE);
		list_add_tail(&free, &p->pages);
	}

	/* Mark pages that we're using to hold page-structs as in-use */
	for (i = 0; i < r; i++) {
		int j = i + (ROUND_DOWN(table, PAGE_SIZE) / PAGE_SIZE);
		struct page* p = &parray[j];
		p->flags = P_IMMUTABLE | P_PHYS | P_KERNEL;
		/* Remove from free list */
		list_del(&p->pages);
	}
	lock_init(&page_lock);

	dprintf("[phys] %d pages in system, %x highest address\n", numpages, memory);
	dprintf("[phys] marked pages from %#x to %#x as %x\n", 
		(ROUND_DOWN(table, PAGE_SIZE)), 
		(ROUND_DOWN(table, PAGE_SIZE)+r*PAGE_SIZE),
		P_IMMUTABLE | P_PHYS | P_KERNEL);

}

/**
 * @brief Mark a range of physical pages with flags, move to proper list.
 * @details Mark all pages between start and end with flags. Valid flags can 
 * be found in "frame.h"
 * 
 * @param start Beginning physical address.
 * @param end Ending physical address.
 * @param flags Page flags.
 * @returns -1 on failure, 0 on success.
 */
int page_mark_range(size_t start, size_t end, int flags)
{
	if (start > end) {
		return -1;
	} else if ((flags & (P_USED|P_FREE)) == (P_USED | P_FREE)) {
		return -1;
	} else if ((flags & (P_USED|P_FREE)) == 0) {
		return -1;
	}

	/* Generate indices into page array */
	size_t s = ROUND_DOWN(start, PAGE_SIZE) / PAGE_SIZE;
	size_t e = ROUND_UP(end, PAGE_SIZE) / PAGE_SIZE;
	assert(e <= numpages);

	for (; s < e; s ++) {
		struct page* p = &parray[s];

		/* Do not allow changing of physical page flags */
		if (p->flags & P_PHYS) 
			continue;

		p->flags = flags;
		/* Move to appropriate list */
		if (flags & P_USED) {
			list_move_tail(&used, &p->pages);
			list_move_tail(&lru, &p->lru);
		} else {
			list_move_tail(&free, &p->pages);
		}		
	}
	dprintf("[phys] marked pages from %#x to %#x as %x\n", start, end, flags);
	return 0;
}

/**
 * @brief Allocate a physical page.
 * @details Lock the page lists. Move page from free list to used list, and move 
 * it to the end of the LRU list.
 * @return NULL on failure. Valid struct page on success
 */
struct page* page_alloc(void)
{
	/* TODO: Sleep until free page, or swap out pages */
	if (list_empty(&free))
		return NULL;

	lock_acquire(&page_lock);
	struct page* p = list_first_entry(&free, struct page, pages);
	list_add_tail(&lru, &p->lru);
	list_move(&used, &p->pages);
	lock_release(&page_lock);

	return p;
}

/**
 * @brief Mark a page as free.
 * @details Lock page array and move lists.
 */
void page_free(struct page* p)
{
	lock_acquire(&page_lock);
	list_move_tail(&free, &p->pages);
	list_del(&p->lru);
	lock_release(&page_lock);
	dprintf("[phys] freeing page %#x\n", p->address);
}

/**
 * @brief Request page struct for a given physical address.
 */
struct page* page_request(size_t address)
{
	size_t index = ROUND_DOWN(address, PAGE_SIZE) / PAGE_SIZE;
	assert(index <= numpages);
	return &parray[index];
}

void page_lru(void)
{
	struct list* l;
	list_for_each(l, &lru) {
		struct page* p = list_entry(l, struct page, lru);
		printf("lru %#x\n", p->address);
	}
}

void page_test(void)
{
	int i;
	for (i = 0; i < 512; i++) {
		struct page* p = page_alloc();
		dprintf("got page %#x\n", p->address);
	}
	//page_lru();
	printf("Testing page request...\n");
	size_t addr = 0x200000	;
	printf("%x, %x\n", addr, page_request(addr)->address);
}
