/*
MIT License

Copyright (c) Michael Lazear, 2016 

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
#include <common.h>
#include <buddy.h>

static int allocated = 0;
static struct buddy* __buddy_root;

/* We need a way to bootstrap our physical memory manager, so we create a 
 * bootstrap allocator. Given a couple physical pages, it will get us up and 
 * running, until the buddy allocator can allocate itself 
 * No need for a bitmap here, we won't free the first couple pages */
static struct heap {
	size_t heap_start;
	size_t heap_end;
	size_t heap_pos;
} bstrap;

static void __bootstrap_init(size_t start, size_t size) {
	bstrap.heap_start = start;
	bstrap.heap_end = start + size;
	bstrap.heap_pos = start;
}

static void* __bootstrap_alloc(size_t size) { 
	if (bstrap.heap_pos + size > bstrap.heap_end)
		return NULL;
	void* ret = (void*) bstrap.heap_pos;
	bstrap.heap_pos += size;
	allocated++;
	return ret;
}

static void* __buddy_internal_alloc() {
	return __bootstrap_alloc(sizeof(struct buddy));

	printf("allocating from slab....\n");
	allocated++;
	return NULL;
}

/* returns zero-index into free lists of the smallest power-of-2 sized block
 * that requested size will fit into */
static int find_smallest_size(int requested) {
	if (!requested)
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

static int buddy_split_block(struct buddy* b) {
	struct buddy* left;
	struct buddy* right;
	struct buddy* tmp;
	int m = b->size - 1;
	if (b->size == MIN_BLOCK_SIZE) {
		printf("min size\n");
		return -1;
	}
	if (b->flags & MASK)
		return -1;
	if (!b->left) {
		left 			= __buddy_internal_alloc();
		assert(left);
		left->size 		= m;
		left->flags 	= m;
		left->addr 		= b->addr;
		left->parent 	= b;
		b->left 		= left;
	}	
	if (!b->right) {
		right 			= __buddy_internal_alloc();
		assert(right);
		right->size 	= m;
		right->flags	= m;
		right->addr 	= (b->left->addr ^ (1 << b->left->size));
		right->parent 	= b;
		b->right 		= right;
	}
	b->flags =  m;
	return 0;
}


/* tail-call optimized tree-recursive buddy allocator */
static struct buddy* __buddy_allocate(struct buddy* start, size_t size) {
	int i = find_smallest_size(size);
	struct buddy* this = start;
	if ((i < MIN_BLOCK_SIZE) || (i > MAX_BLOCK_SIZE)) 
		return NULL;

tco:
	if (start->left && start->right) {
		if (!FIT(start->left, i) && !FIT(start->right, i))
			return NULL;
	}

	/* We have a match! */
	if (this->size == i && !(this->flags & MASK)) {
		this->flags |= INUSE;
		this->flags &= ~S_MASK;
		dprintf("[buddy] allocating block of size %#x at address %#x\n", 1 << this->size, this->addr);
		return this;
	}

	/* No match in this block, try to split and traverse */
	buddy_split_block(this);

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
	goto tco;
}


/* buddy_block_from_addr will try to reach the block represented by the 
 * requested address and size.
 * if the downstream block does not exist, and the upstream block doesn't 
 * already have children, then they will be spawned 
 * returns NULL if the block is inaccessible (i.e. downstream of a larger block
 * with no children) */
static struct buddy* __buddy_from_addr(struct buddy* start, size_t address, size_t size) {
	if (!start)
		return NULL;
	struct buddy* this = start;
	while(size < this->size) {
		assert(this);
		/* If there are no children, try to create them */
		if (!this->left || !this->right)
			if (buddy_split_block(this))
				break;
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

struct buddy* buddy_get(size_t address, size_t size) {
	return __buddy_from_addr(__buddy_root, address, size);
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
	__bootstrap_init(reserved, ressz);
	printf("Initializing buddy allocator. 0x%08x system memory\n", system_memory);
	printf("Bootstrapping @ 0x%08x-%08x\n", reserved, ressz);
	/* calculate total amount of memory */
	uint64_t i = 0;
	while (system_memory /= 2) {
		i++;
	}
	__buddy_root 			= __buddy_internal_alloc();
	__buddy_root->size 		= i;
	__buddy_root->addr 		= 0;
	__buddy_root->parent 	= NULL;
	__buddy_root->flags 	= 0;

		/* Now we abuse the system to get a chunk > MAX_BLOCK_SIZE for the buddy
	 * tree itself. Because if you can't abuse the rules you yourself made, why
	 * even make rules? */
	//struct buddy* slab = buddy_get(0, 25);
	/* Carve out the first 4MB for kernel space, no one gets to touch this */
	//struct buddy* kernel = __buddy_allocate(slab, (1<<21));

	/* Manually mark the right half of the slab space as in use, the left half
	 * that is greater than 0x00400000 and less than 0x04000000 is now free 
	 * for allocation */
	//slab = slab->right;

	//slab->flags |= INUSE;
//	__bootstrap_slab(slab);

}

struct buddy* buddy_alloc(int size) {
	return __buddy_allocate(__buddy_root, size);
}
