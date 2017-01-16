#ifndef __LOCK__
#define __LOCK__

#include <stdatomic.h>

struct lock {
	atomic_flag lock;
};

static inline void lock_init(struct lock* lock) {
	atomic_flag_clear(&lock->lock);
}

static inline void lock_acquire(struct lock* lock) {
	while(atomic_flag_test_and_set(&lock->lock))
		asm("pause");
}

static inline void lock_release(struct lock* lock) {
	atomic_flag_clear(&lock->lock);
}

#endif