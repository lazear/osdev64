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

#ifndef __BUDDY__
#define __BUDDY__

/* block size is log2.
 * i.e. 1 << 12 == 0x00001000
 *		1 << 22 == 0x00400000
 */
#define MIN_BLOCK_SIZE	12	/* four kilobyte sized blocks */
#define MAX_BLOCK_SIZE 	23	/* four megabyte sized blocks */


#define S_MASK 		0x01F
#define LEFT_USED 	0x200
#define RIGHT_USED 	0x400
#define INUSE		0x800
#define LR_MASK 	0x600
#define MASK 		0xE00

#define FIT(s, c) 	(((((s)->flags) & S_MASK) >= c) && (!((s)->flags & INUSE)))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define FREE(a)		(((a)->flags & S_MASK) == (a)->size)

struct buddy {
	size_t addr;			/* address that this block represents */
	size_t size;
	size_t flags;	

	struct buddy* parent;	/* buddy's parent */
	struct buddy* left;		/* left node */
	struct buddy* right;	/* right node */
} __attribute__((packed));

extern void buddy_initialize(uint64_t, size_t, size_t);
extern struct buddy* buddy_alloc(int size);
extern void buddy_free(struct buddy* block);
extern struct buddy* buddy_get(size_t address, size_t size);


#endif