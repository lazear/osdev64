/*
MIT License

Copyright (c) Michael Lazear, 2016-2017

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
#include <stdio.h>
#include <assert.h>
#include <buddy.h>
#include <list.h>

static int allocated = 0;
static struct blist {
	struct buddy* b;
	struct blist* prev;
	struct blist* next;
} list;

/* We need a way to bootstrap our physical memory manager, so we create a 
 * bootstrap allocator. Given a couple physical pages, it will get us up and 
 * running, until the buddy allocator can allocate itself 
 * No need for a bitmap here, we won't free the first couple pages */
static struct heap {
	size_t heap_start;
	size_t heap_end;
	size_t heap_pos;
} bstrap;

static void bootstrap_init(size_t start, size_t size) {
	bstrap.heap_start = start;
	bstrap.heap_end = start + size;
	bstrap.heap_pos = start;
}

static void* bootstrap_alloc(size_t size) { 
	if (bstrap.heap_pos + size > bstrap.heap_end)
		return NULL;
	void* ret = (void*) bstrap.heap_pos;
	bstrap.heap_pos += size;
	allocated++;
	return ret;
}

static void* buddy_internal_alloc() {
	return bootstrap_alloc(sizeof(struct buddy));

	allocated++;
	return NULL;
}

/**
 * @brief Returns smallest log2-sized value that will accomodate the requested
 * size.
 * @details Requested size must be within valid request size bounds 
 * MIN_BLOCK_SIZE and MAX_BLOCK_SIZE.
 * 
 * @param requested Number of bytes requested.
 * @return Smallest log2-sized block that will fit request.
 */
static int find_smallest_size(int requested) {
	if (requested <= 0)
		return -1;
	for (int i = MIN_BLOCK_SIZE; i < MAX_BLOCK_SIZE; i++) {
		int mask = (1 << i);
		if (requested < mask || !(requested & ~mask))
			return i ;
		if (((requested & ~mask) < mask))
			return i + 1;
	}
	return -1;
}

/**
 * @brief Given a block, split it into two equally sized children.
 * @details Create left and right children blocks. Both children 
 * are half the size of the parent block. Parent has it's size
 * mask (S_MASK) value in buddy->flags set to the block size of
 * the children.
 * 
 * @param buddy Parent block.
 * @param force Force allocation of children on INUSE blocks.
 * @return 0 on success, -1 on failure.
 */
static int buddy_split_block(struct buddy* b, int force) {
	struct buddy* left;
	struct buddy* right;
	struct buddy* tmp;
	/* Log2 sizes, so decrease by 1 */
	int m = b->size - 1;
	if (b->size == MIN_BLOCK_SIZE) {
		printf("min size\n");
		return -1;
	}
	if ((b->flags & MASK) && (force == 0))
		return -1;
	if (!(b->flags & GOOD))
	if (!b->left) {
		left 			= buddy_internal_alloc();
		assert(left);
		left->size 		= m;
		left->flags 	= m | GOOD;
		left->addr 		= b->addr;
		left->parent 	= b;
		b->left 		= left;
	}	
	if (!b->right) {
		right 			= buddy_internal_alloc();
		assert(right);
		right->size 	= m;
		right->flags	= m | GOOD;
		right->addr 	= (b->left->addr ^ (1 << b->left->size));
		right->parent 	= b;
		b->right 		= right;
	}
	b->flags =  m;
	return 0;
}


/**
 * @brief Tail-call optimized recursive buddy allocator.
 * @details Given a tree root, search for a buddy with a block that will fit
 * the requested size. Calls buddy_split_block to generate new buddys when
 * needed
 * 
 * @param buddy Root of the buddy tree.
 * @param size Size, in bytes.
 * 
 * @return NULL on error, or a valid buddy block.
 */
static struct buddy* __buddy_allocate(struct buddy* start, size_t size) {
	int i = find_smallest_size(size);
	struct buddy* this = start;
	if ((i < MIN_BLOCK_SIZE) || (i > MAX_BLOCK_SIZE)) 
		return NULL;
	// if ((i > start->size) || (i > (start->flags & S_MASK)))
	// 	return NULL;
find:
	if (start->left && start->right) {
		if (!FIT(start->left, i) && !FIT(start->right, i))
			return NULL;
	}

	/* We have a match! */
	if (this->size == i && !(this->flags & MASK) && (this->flags & GOOD)) {
		this->flags |= INUSE;
		this->flags &= ~S_MASK;
		//printf("[buddy] allocating block of size %#x at address %#x\n", 1 << this->size, this->addr);
		return this;
	}

	/* No match in this block, try to split and traverse */
	buddy_split_block(this, 0);

	/* Mark as we go down the tree */
	if (this->left->flags & INUSE)
		this->flags |= LEFT_USED;
	if (this->right->flags & INUSE)
		this->flags |= RIGHT_USED;
	if ((this->flags & LR_MASK) == LR_MASK) 
		this->flags = INUSE;

	/* Check to see if there is a free block of requested size in either the 
	 * left or the right leaf of the node */
	if (FIT(this->left, i)) {
		this = this->left;
	} else if (FIT(this->right, i))  {
		this = this->right;
	} else {
		/* Neither the left nor the right leaves can fit a block of the 
		 * requested size, so we clear our size field, and then set it to the 
		 * size of the left or right leaf largest available block */
		this->flags &= ~S_MASK;
		this->flags |= MAX((this->left->flags & S_MASK), (this->right->flags & S_MASK));
		this = this->parent;
	}
	goto find;
}

 /**
  * @brief Attempt to return the block represented the requested address and 
  * size.
  * @details If the downstream block does not exist, and the upstream block 
  * doesn't already have children, then they will be spawned. Returns NULL if 
  * the block is inaccessible (i.e. downstream of a larger block with no 
  * children that has been allocated)
  * 
  * @param buddy Tree root
  * @param address Requested address
  * @param size log2 of size.
  * @return NULL on failure, or a valid struct buddy.
  */
static struct buddy* buddy_from_addr(struct buddy* start, size_t address, size_t size) {
	if (!start)
		return NULL;
	struct buddy* this = start;
	while(size < this->size) {
		assert(this);
		/* If there are no children, try to create them */
		if (!this->left || !this->right) {
			if (buddy_split_block(this, 0))
				break;
		}
		assert (this->left || this->right);

		if (address < this->addr + (1 << (this->size - 1)))	
			this = this->left;
		else
			this = this->right;
	}
	if (this->addr == address && this->size == size)
		return this;
	printf("bad block! %x-%x %x\n", this->addr, 1 << this->size, this->flags);
	return NULL;
}

struct buddy* buddy_add_range(size_t start, size_t end)
{
	printf("[buddy] adding address range from %x to %x\n", start, end);
	size_t size = find_smallest_size(end-start);

	struct buddy* range = buddy_internal_alloc();
	range->size = size;
	range->flags = size;

	struct blist* a = &list;
	struct blist* b = bootstrap_alloc(sizeof(struct blist));
	(&list)->prev = b;
	b->next = &list;
	b->prev = (&list)->prev;
	(&list)->prev->next = b;
}

void b_list(void)
{
	printf("BLIST\n");
	struct blist* a;
	for(a = list.next; a != &list; a = a->next)
			printf("list addr %x - %x\n", a->b->addr, a->b->addr + (1 << a->b->size));

}

void buddy_free(struct buddy* block) {
	struct buddy* tmp 	= block;
	int which			= (block == block->parent->left) ? LEFT_USED : RIGHT_USED;
	int sz 				= block->size;

	if (block->left && block->right) {
		if (block->left->flags & MASK)
			printf("(BLOCK LEFT HAS FLAGS! %X)\n", block->left->flags);
		if (block->right->flags & MASK)
			printf("(BLOCK right HAS FLAGS! %X)\n", block->right->flags);		
	}

	/* clear all flags, set size mask */
	block->flags = block->size;	
	tmp = block->parent;
	sz = block->size;
	/* Go up the tree and coalesce blocks */
	for (tmp = block->parent; tmp->parent; tmp = tmp->parent) {
		tmp->flags &= ~(INUSE | which);
		/* If new size is larger, then overwrite*/
		if ((tmp->flags & S_MASK) < sz) {
			tmp->flags &= ~S_MASK;
			tmp->flags |= sz;
		}
		/* Both children are free*/
		if (FREE(tmp->left) && FREE(tmp->right)) {
			/* There is now a new bigger sized block, so update the size mask
			 * to be propagated */
			sz = tmp->size;
			tmp->flags = sz;

			/* TODO: free children to cache or something */
		}
		which = (tmp == tmp->parent->left) ? LEFT_USED : RIGHT_USED;
	}

}

void buddy_initialize(uint64_t system_memory, size_t reserved, size_t ressz) {
	printf("Initializing buddy allocator. 0x%x system memory\n", system_memory);
	bootstrap_init(reserved, ressz);
	list.next = &list;
	list.prev = &list;
	printf("Bootstrapping @ 0x%08x-%08x\n", reserved, ressz);
	// /* calculate total amount of memory */
	// uint64_t i = 0;

	// while (system_memory /= 2) {
	// 	i++;
	// }

	// struct buddy_root = buddy_internal_alloc();

	// buddy_root->size 		= i;
	// buddy_root->addr 		= 0;
	// buddy_root->parent 	= NULL;
	// buddy_root->flags 	= MASK;
	// printf("[buddy] root @ %x, size %x\n", buddy_root->addr, buddy_root->size);
	// 	/* Now we abuse the system to get a chunk > MAX_BLOCK_SIZE for the buddy
	//  * tree itself. Because if you can't abuse the rules you yourself made, why
	//  * even make rules? */
	// struct buddy* slab = buddy_get(0, 15);
	// assert(slab);

	// printf("slab size %x @ %x\n", 1 << slab->size, slab->addr);
	/* Carve out the first 4MB for kernel space, no one gets to touch this */
	//struct buddy* kernel = __buddy_allocate(slab, (1<<21));

	/* Manually mark the right half of the slab space as in use, the left half
	 * that is greater than 0x00400000 and less than 0x04000000 is now free 
	 * for allocation */
	//slab = slab->right;

	//slab->flags |= INUSE;
//	__bootstrap_slab(slab);

}

/**
 * @brief Return a struct buddy matching request, without allocating.
 * 
 * @param address 	Requested address.
 * @param size 		Requested log2 of size.
 */
struct buddy* buddy_get(size_t address, size_t size) {
	return buddy_from_addr(list.b, address, size);
}


/**
 * @brief Allocate a new block of memory.
 * 
 * @param size Size in bytes of requested page.
 * @return struct buddy.
 */
struct buddy* buddy_alloc(int size) {
	return __buddy_allocate(list.b, size);
}
