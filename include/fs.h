
#ifndef __FS__
#define __FS__

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>


struct file {

	uint16_t mode;			/**< File mode */
	uint32_t flags;			/**< File flags */
	size_t pos;			/**< Current read/write position */
	void* private;			/**< Optional private data */
	const struct file_operations* ops;

} __attribute__((aligned(4)));

/**
 * @brief      Pointers to functions dealing with files
 */
struct file_operations {
	int (*open) (void*, struct file*);
	int (*close) (struct file*);
	size_t (*read) (struct file*, char* __user, size_t cnt);
	size_t (*write) (struct file*, char* __user, size_t cnt);
};


#endif