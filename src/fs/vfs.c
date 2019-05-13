#include <elfboot/vfs.h>

#include <list.h>
#include <tree.h>

TREE_HEAD(vfs_mountpoints);

static void __unused vfs_add_mountpoint(struct tree_node *new, struct tree_node *parent)
{
	struct tree_node *pos;

	new->parent = parent;

	if (tree_leaf(parent)) {
		list_add(&new->siblings, &parent->children);
		goto vfs_inc_count;
	}

	tree_for_each_child(pos, parent) {
		if (strcmp(new->data, pos->data) < 0) {
			if (pos->siblings.prev == &parent->children)
				list_add(&new->siblings, &parent->children);
			else
				list_add(&new->siblings, pos->siblings.prev);

			goto vfs_inc_count;
		}
	}

	list_add(&new->siblings, parent->children.prev);

vfs_inc_count:

	parent->children_count++;
	vfs_mountpoints.count++;
}

static void __unused vfs_del_mountpoint(struct tree_node *node)
{
	struct tree_node *pos;

	/*
	 * Contrary to general tree structures, the removal of a node inside
	 * the vfs mountpoint tree structure also leads to the removal of its
	 * children.
	 */
	
	if (!tree_leaf(node)) {
		tree_for_each_child(pos, node) {

			/* Recursively delete all child nodes */
			vfs_del_mountpoint(pos);
			vfs_mountpoints.count--;

			/* Free the current nodes mount data */
			bfree(pos->data);
		}
	}

	node->parent->children_count += node->children_count - 1;

	list_del(&node->children);
	list_del(&node->siblings);

	vfs_mountpoints.count--;

	/* Free the node itself */
	bfree(node);
}

void vfs_mount(const char *path __unused, struct device *device __unused)
{
	
}