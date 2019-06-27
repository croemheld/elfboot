#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <uapi/elfboot/common.h>

struct list_head {
	struct list_head *prev, *next;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

#define list_entry(ptr, type, member)                                          \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member)                                    \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member)                                     \
    list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member)                                           \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head)                                               \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)                                       \
    for (pos = (head)->next, n = pos->next;                                    \
        pos != (head);                                                         \
        pos = pos->next, n = pos->next)

#define list_for_each_entry(pos, head, member)                                 \
    for (pos = list_entry((head)->next, typeof(*pos), member);                 \
        &pos->member != (head);                                                \
        pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)                         \
    for (pos = list_first_entry(head, typeof(*pos), member),                   \
            n = list_next_entry(pos, member);                                  \
        &pos->member != (head);                                                \
        pos = n, n = list_next_entry(n, member))

#define list_for_each_prev(pos, head)                                          \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_first_entry_or_null(ptr, type, member)                            \
({                                                                             \
    struct list_head *head__ = (ptr);                                          \
    struct list_head *pos__ = head__->next;                                    \
    pos__ != head__ ? list_entry(pos__, type, member) : NULL;                  \
})

#define LIST_HEAD_INIT(name)						\
	{ &(name), &(name) }

#define LIST_HEAD(name)							\
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *head)
{
	head->prev = head->next = head;
}

#define HLIST_HEAD_INIT							\
	{ .first = NULL }

#define HLIST_HEAD(name)						\
	struct hlist_head name = HLIST_HEAD_INIT

static inline void INIT_HLIST_NODE(struct hlist_node *node)
{
	node->next = NULL;
	node->pprev = NULL;
}	

static inline int __list_add_valid(struct list_head *new, struct list_head *prev, 
	struct list_head *next)
{
	if(prev->next != next)
		return 0;

	if(next->prev != prev)
		return 0;

	if(new == prev || new == next)
		return 0;

	return 1;
}

static inline int __list_del_entry_valid(struct list_head *entry)
{
	struct list_head *prev = entry->prev;
	struct list_head *next = entry->next;

	if(prev == NULL)
		return 0;

	if(next == NULL)
		return 0;

	if(prev->next != entry)
		return 0;

	if(next->prev != entry)
		return 0;

	return 1;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, 
	struct list_head *next)
{
	if (!__list_add_valid(new, prev, next))
		return;

	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void __list_del_entry(struct list_head *entry)
{
	if (!__list_del_entry_valid(entry))
		return;

	__list_del(entry->prev, entry->next);
}

static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct list_head *list, 
	struct list_head *prev, struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void list_init(struct list_head *head)
{
	head->prev = head;
	head->next = head;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

static inline void list_del(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if(!list_empty(list))
		__list_splice(list, head, head->next);
}

#endif /* __LIST_H__ */