#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

#include <fs/isofs.h>

/*
 * List of available file systems
 */
LIST_HEAD(filesystems);

/*
 * Filesystem root node
 */
static struct fs_node *fs_root;

struct fs_node *fs_node_alloc(struct fs *fs)
{
	struct fs_node *node;

	node = bmalloc(sizeof(*node));
	if (!node)
		return NULL;

	tree_node_init(&node->fs_node);

	node->ops = fs->n_ops;

	return node;
}

/* 
 * fs_node functions
 */

static int fs_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int fs_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}


/* 
 * fs_node functions for directories
 */

static struct fs_node *fs_lookup(struct fs_node *node, const char *path)
{
	char *name, *part;

	if (!node || *path != '/')
		return NULL;

	if (strlen(path) == 1)
		return fs_root;

	name = bstrdup(path);
	if (!name)
		return NULL;

	part = strtok(name, "/");

	while (part) {
		if (!node->ops->lookup) {
			bfree(name);
			return NULL;
		}

		node = node->ops->lookup(node, part);
		if (!node) {
			bfree(name);
			return NULL;
		}

		part = strtok(part, "/");
	}

	bfree(name);

	return node;
}

static int fs_readdir(struct fs_node *node __unused, struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}


/* 
 * fs_node function for files
 */

static int fs_read(struct fs_node *node __unused, uint64_t sector __unused,
		    char *buffer __unused)
{
	return -ENOTSUP;
}

static int fs_write(struct fs_node *node __unused, uint64_t sector __unused,
		     const char *buffer __unused)
{
	return -ENOTSUP;
}

/* -------------------------------------------------------------------------- */

static int fs_mount_fs(struct device *device, struct fs *fs, const char *path)
{
	struct fs_node *parent, *child;

	/*
	 * The mounting process takes the given path and creates 
	 * a new node with the device name in its directory.
	 *
	 * The mounted device is then available via:
	 * 	
	 * 	 path + "/" + device->name
	 */
	
	parent = fs_lookup(fs_root, path);
	if (!parent) {
		bprintln("Could not find node for %s", path);
		return -EFAULT;
	}

	child = device->sb->root;

	/* Copy device name for node */
	strcpy(child->name, device->name);

	/* The new node is both a mountpoint and a directory */
	fs_node_set(child, FS_NODE_MOUNT | FS_NODE_DIRECTORY);

	/* Insert the node in the VFS tree */
	tree_node_insert(&child->fs_node, &parent->fs_node);

	return 0;
}

int fs_mount(struct device *device, const char *path)
{
	struct fs *fs;

	list_for_each_entry(fs, &filesystems, list) {
		if (superblock_probe(device, fs))
			continue;

		return fs_mount_fs(device, fs, path);
	}

	return -EFAULT;
}

void fs_register(struct fs *fs)
{
	list_add(&fs->list, &filesystems);
}

void fs_unregister(struct fs *fs)
{
	list_del(&fs->list);
}

void fs_init(void)
{
	isofs_fs_init();
}