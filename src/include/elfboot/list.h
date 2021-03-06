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

#define list_entry(ptr, type, member)					\
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member)				\
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member)				\
	list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member)					\
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head)					\
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)				\
	for (pos = (head)->next, n = pos->next;				\
	     pos != (head);						\
	     pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     &pos->member != (head);					\
	     pos = n, n = list_next_entry(n, member))

#define list_for_each_prev(pos, head)					\
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_first_or_null(ptr)							\
({														\
	(ptr)->next != (ptr) ? (ptr)->next : NULL;			\
})

#define list_first_entry_or_null(ptr, type, member)			\
({									\
	struct list_head *head__ = (ptr);				\
	struct list_head *pos__ = head__->next;				\
	pos__ != head__ ? list_entry(pos__, type, member) : NULL;	\
})

#define LIST_HEAD_INIT(name)						\
	{ &(name), &(name) }

#define LIST_HEAD(name)							\
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *head)
{
	head->prev = head->next = head;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, 
	struct list_head *next)
{
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
	__list_del(entry->prev, entry->next);
	entry->prev = entry->next = NULL;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add(list, head);
}

static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}

/*
 * hlist_head and hlist_node for hashtable buckets
 */

struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define hlist_for_each_safe(pos, n, head)				\
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); 	\
	     pos = n)

#define hlist_entry_safe(ptr, type, member)				\
	({ typeof(ptr) ____ptr = (ptr); 				\
	    ____ptr ? hlist_entry(____ptr, type, member) : NULL;	\
	})

#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry_safe((head)->first,			\
		typeof(*(pos)), member);				\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next,			\
		typeof(*(pos)), member))

#define hlist_for_each_entry_continue(pos, member)			\
	for (pos = hlist_entry_safe((pos)->member.next,			\
		typeof(*(pos)), member);				\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next,			\
		typeof(*(pos)), member))

#define hlist_for_each_entry_from(pos, member)				\
	for (; pos;							\
	     pos = hlist_entry_safe((pos)->member.next,			\
		typeof(*(pos)), member))

#define hlist_for_each_entry_safe(pos, n, head, member)			\
	for (pos = hlist_entry_safe((head)->first,			\
		typeof(*pos), member);					\
	     pos && ({ n = pos->member.next; 1; });			\
	     pos = hlist_entry_safe(n, typeof(*pos), member))

#define HLIST_HEAD_INIT							\
	{ .first = NULL }

#define HLIST_HEAD(name)						\
	struct hlist_head name = HLIST_HEAD_INIT

static inline void INIT_HLIST_NODE(struct hlist_node *node)
{
	node->next = NULL;
	node->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *node)
{
	return !node->pprev;
}

static inline int hlist_empty(const struct hlist_head *head)
{
	return !head->first;
}

static inline void __hlist_del(struct hlist_node *node)
{
	struct hlist_node *next = node->next;
	struct hlist_node **pprev = node->pprev;

	*pprev = next;

	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *node)
{
	__hlist_del(node);
	node->next = NULL;
	node->pprev = NULL;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node *node, 
				  struct hlist_head *head)
{
	struct hlist_node *first = head->first;

	node->next = first;

	if (first)
		first->pprev = &node->next;

	head->first = node;
	node->pprev = &head->first;
}

static inline void hlist_add_before(struct hlist_node *node,
				    struct hlist_node *next)
{
	node->pprev = next->pprev;
	node->next = next;
	next->pprev = &node->next;
	*(node->pprev) = node;
}

static inline void hlist_add_behind(struct hlist_node *node,
				    struct hlist_node *prev)
{
	node->next = prev->next;
	prev->next = node;
	node->pprev = &prev->next;

	if (node->next)
		node->next->pprev = &node->next;
}

#endif /* __LIST_H__ */