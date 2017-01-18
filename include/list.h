/* Adapted from linux */

#ifndef __LIST__
#define __LIST__

/**
 * @brief      Generic doubly linked circular list
 */
struct list_head {
	struct list_head* next, *prev;
};


#define LIST_HEAD_INIT(name) { &(name), &(name) }
/**
 * @brief      Declare and initialize a new struct list_head.
 */
#define LIST_HEAD(name)	struct list_head name = LIST_HEAD_INIT(name)

/**
 * @brief      Access the structure in which ptr is a member.
 * @param      ptr     list_head* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list_head element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_entry(ptr, type, member) \
	((type*) ((char*) (ptr) - (unsigned long)(&((type*)0)->member)))

/**
 * @brief      Iterate over a struct list_head.
 * @param      pos   (list_head*) iterator
 * @param      head  (list_head*) list
 */
#define list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief      Iterate over a struct list_head, use for deletion or insertion.
 * @param      pos   (list_head*) iterator
 * @param      head  (list_head*) list
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * @brief      Access the container structure of the last entry in the list.
 * @param      ptr     list_head* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list_head element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * @brief      Access the container structure of the first entry in the list.
 * @param      ptr     list_head* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list_head element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

static inline void list_init(struct list_head* list) {
	list->next = list;
	list->prev = list;
}

static inline int list_empty(struct list_head* list) {
	return (list->next == list);
}

static inline void __list_del(struct list_head* prev, struct list_head* next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head* entry) {
	__list_del(entry->prev, entry->next);
}


static inline void list_replace(struct list_head *old, struct list_head *new) {
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next) {
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * @brief      Insert new at the beginning of head
 */
static inline void list_add(struct list_head* head, struct list_head* new) {
	__list_add(new, head, head->next);
}

/**
 * @brief      Insert new at the tail of head
 */
static inline void list_add_tail(struct list_head* head, struct list_head* new) {
	__list_add(new, head->prev, head);
}

/**
 * @brief      Remote entry from it's current list, and move it to the front of list.
 */
static inline void list_move(struct list_head* list, struct list_head* entry) {
	list_del(entry);
	list_add(list, entry);
}

/**
 * @brief      Remote entry from it's current list, and move it to the tail of list.
 */
static inline void list_move_tail(struct list_head* list, struct list_head* entry) {
	list_del(entry);
	list_add_tail(list, entry);
}




#endif