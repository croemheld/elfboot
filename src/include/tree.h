#ifndef __TREE_H__
#define __TREE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <list.h>

#include <uapi/elfboot/common.h>

struct tree_node {
	void *data;
	struct tree_node *parent;
	struct list_head children;
	struct list_head siblings;
	uint32_t children_count;
};

struct tree_head {
	int count;
	struct tree_node *root;
};

#define tree_for_each_child(pos, parent)				\
	list_for_each_entry(pos, &(parent)->children, siblings)

#define tree_for_each_child_safe(pos, n, parent)			\
	list_for_each_entry_safe(pos, n, &(parent)->children, siblings)

#define TREE_HEAD_INIT(name)						\
	{ 0, NULL }

#define TREE_HEAD(name)							\
	struct tree_head name = TREE_HEAD_INIT(name)

/*
 * General n-ary tree functions
 */

static inline int tree_leaf(struct tree_node *node)
{
	return !node->children_count;
}

#endif /* __TREE_H__ */