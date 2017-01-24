/* physical memory manager */
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <list.h>

struct page {
	uint64_t address;
	int flags;
	atomic_int refs;
	struct list_head lru;
	struct list_head list;
};

static LIST_HEAD(free);
static LIST_HEAD(used);
static LIST_HEAD(lru);


void physical_mem_init(void)
{
	printf("sizeof struct page: %x bytes %d/page\n", sizeof(struct page), 0x1000/sizeof(struct page));
}

void mem_add_range(uint64_t	address, uint64_t length)
{

}
