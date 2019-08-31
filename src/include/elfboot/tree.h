#ifndef __TREE_H__
#define __TREE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

struct tree_node;

struct tree_ops {
	int (*insert)(struct tree_node *, struct tree_node *);
	int (*remove)(struct tree_node *);
};

struct tree_node {
	struct tree_node *parent;
	struct list_head children;
	struct list_head siblings;
	uint32_t children_count;
};

struct tree_head {
	int count;
	struct tree_ops *ops;
	struct tree_node *root;
};

#define tree_entry(node, type, member)					\
	container_of(node, type, member)

/*
 * Parent
 */

#define tree_parent(node)						\
	((node)->parent)

#define tree_parent_entry(node, type, member)				\
	tree_entry(tree_parent(&(node)->member), type, member)

/*
 * First and last child
 */

#define tree_first_child(node)						\
	tree_entry((node)->children.next, struct tree_node, siblings)

#define tree_first_child_or_null(node)					\
({									\
	struct list_head *p__ = &(node)->children;			\
	struct list_head *n__ = (node)->children.next;			\
	n__ != p__ ? tree_first_child(node) : NULL;			\
})

#define tree_first_child_entry(node, type, member)			\
	tree_entry(tree_first_child(node), type, member)		\

#define tree_first_child_entry_or_null(node, type, member)		\
({									\
	struct tree_node *n__;						\
	n__ = tree_first_child_or_null(node);		 		\
	n__ != NULL ? tree_entry(n__, type, member) : NULL;		\
})

#define tree_last_child(node)						\
	tree_entry((node)->children.prev, struct tree_node, siblings)

#define tree_last_child_entry(node, type, member)			\
	tree_entry(tree_last_child(node), type, member)

/*
 * Next and previous sibling
 */

#define tree_next_sibling(node)						\
	tree_entry((node)->siblings.next, struct tree_node, siblings)

#define tree_next_sibling_or_null(node)					\
({									\
	struct list_head *p__ = &(node)->parent->children;		\
	struct list_head *n__ = (node)->siblings.next;			\
	n__ != p__ ? tree_next_sibling(node) : NULL;			\
})

#define tree_next_sibling_entry(node, type, member)			\
	tree_entry(tree_next_sibling(node), type, member)

#define tree_next_sibling_entry_or_null(node, type, member)		\
({									\
	struct tree_node *n__;						\
	n__ = tree_next_sibling_or_null(node);				\
	n__ != NULL ? tree_entry(n__, type, member) : NULL;		\
})

#define tree_prev_sibling(node)						\
	tree_entry((node)->siblings.prev, struct tree_node, siblings)

#define tree_prev_sibling_entry(node, type, member)			\
	tree_entry(tree_prev_sibling(node), type, member)

/*
 * Loops
 */

#define tree_for_each_child(pos, node)					\
	for (pos = tree_first_child_or_null(node);			\
	     pos;							\
	     pos = tree_next_sibling_or_null(pos))

#define tree_for_each_child_entry(pos, node, member)			\
	for (pos = tree_first_child_entry_or_null(&(node)->member, 	\
		typeof(*pos), member);					\
	     pos;							\
	     pos = tree_next_sibling_entry_or_null(&(pos)->member,	\
	     	typeof(*pos), member))

#define TREE_HEAD_INIT(name)						\
	{ 0, NULL, NULL }

#define TREE_HEAD(name)							\
	struct tree_head name = TREE_HEAD_INIT(name)

static inline void tree_node_init(struct tree_node *node)
{
	node->parent = NULL;
	list_init(&node->children);
	list_init(&node->siblings);
	node->children_count = 0;
}

/*
 * General n-ary tree functions
 *
 * int tree_node_is_leaf(struct tree_node *node): 
 * 	Is the specified node a leaf node?
 * 	
 * void tree_node_insert(struct tree_node *new, struct tree_node *parent):
 * 	Add a new node with all its children to an 
 * 	existing parent node.
 *
 * void tree_node_remove(struct tree_node *node):
 * 	Remove the specified node from the tree, but 
 * 	keep all references to its children.
 */

static inline int tree_node_is_leaf(struct tree_node *node)
{
	return !node->children_count;
}

static inline void tree_node_insert(struct tree_node *new, 
				    struct tree_node *parent)
{
	parent->children_count += (new->children_count + 1);
	new->parent = parent;
	
	list_add(&new->siblings, &parent->children);
}

static inline void tree_node_remove(struct tree_node *node)
{
	node->parent->children_count -= (node->children_count + 1);
	node->parent = NULL;

	list_del(&node->siblings);
}

#endif /* __TREE_H__ */