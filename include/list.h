/* Adapted from linux */

#ifndef __LIST__
#define __LIST__

/**
 * @brief      Generic doubly linked circular list.
 */
struct list {
	struct list* next, *prev;
};


#define list_INIT(name) { &(name), &(name) }
/**
 * @brief      Declare and initialize a new struct list.
 */
#define list(name)	struct list name = list_INIT(name)

/**
 * @brief      Access the structure in which ptr is a member.
 * @param      ptr     list* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_entry(ptr, type, member) \
	((type*) ((char*) (ptr) - (unsigned long)(&((type*)0)->member)))

/**
 * @brief      Iterate over a struct list.
 * @param      pos   (list*) iterator
 * @param      head  (list*) list
 */
#define list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief      Iterate over a struct list, use for deletion or insertion.
 * @param      pos   (list*) iterator
 * @param      head  (list*) list
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * @brief      Access the container structure of the last entry in the list.
 * @param      ptr     list* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * @brief      Access the container structure of the first entry in the list.
 * @param      ptr     list* pointer
 * @param      type    Type of containing structure
 * @param      member  Name of list element in structure
 * @return     (type*) Pointer to containing structure
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

static inline void list_init(struct list* list) {
	list->next = list;
	list->prev = list;
}

static inline int list_empty(struct list* list) {
	return (list->next == list);
}

static inline void __list_del(struct list* prev, struct list* next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list* entry) {
	__list_del(entry->prev, entry->next);
}


static inline void list_replace(struct list *old, struct list *entry) {
	entry->next = old->next;
	entry->next->prev = entry;
	entry->prev = old->prev;
	entry->prev->next = entry;
}

static inline void __list_add(struct list *entry, struct list *prev, struct list *next) {
	next->prev = entry;
	entry->next = next;
	entry->prev = prev;
	prev->next = entry;
}

/**
 * @brief      Insert new at the beginning of head
 */
static inline void list_add(struct list* head, struct list* entry) {
	__list_add(entry, head, head->next);
}

/**
 * @brief      Insert new at the tail of head
 */
static inline void list_add_tail(struct list* head, struct list* entry) {
	__list_add(entry, head->prev, head);
}

/**
 * @brief      Remote entry from it's current list, and move it to the front of list.
 */
static inline void list_move(struct list* head, struct list* entry) {
	list_del(entry);
	list_add(head, entry);
}

/**
 * @brief      Remote entry from it's current list, and move it to the tail of list.
 */
static inline void list_move_tail(struct list* head, struct list* entry) {
	list_del(entry);
	list_add_tail(head, entry);
}



#endif