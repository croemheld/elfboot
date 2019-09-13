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
#include <fs/ramfs.h>

/*
 * List of available file systems
 */
LIST_HEAD(filesystems);

/*
 * Filesystem root node
 */
static struct fs_node *fs_root;

/*
 * Caches for allocating fs_node and fs_dentry structures
 */
static struct bmem_cache *node_cache;
static struct bmem_cache *dent_cache;

static void fs_node_init(void *objp)
{
	struct fs_node *node = objp;

	tree_node_init(&node->fs_node);
}

struct fs_node *fs_node_alloc(struct fs *fs, const char *name)
{
	struct fs_node *node = bmem_cache_alloc(node_cache);

	if (!node)
		return NULL;

	node->ops = fs->n_ops;
	strcpy(node->name, name);

	return node;
}

static void fs_dent_init(void *objp)
{
	struct fs_dentry *dentry = objp;

	/* Empty name until usage */
	strcpy(dentry->name, "\0");
	dentry->offset = 0;
}

struct fs_dentry *fs_dentry_alloc(const char *name, uint64_t offset)
{
	struct fs_dentry *dentry = bmem_cache_alloc(dent_cache);

	if (!dentry)
		return NULL;

	strcpy(dentry->name, name);
	dentry->offset = offset;

	return dentry;
}

struct fs_dentry *fs_node_dentry_alloc(struct fs_node *node)
{
	return fs_dentry_alloc(node->name, node->offset);
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

static int fs_lookup_fast(struct fs_node **node, const char *name)
{
	struct fs_node *child;

	tree_for_each_child_entry(child, *node, fs_node) {
		if (strcmp(child->name, name))
			continue;

		*node = child;

		return 0;
	}

	return -EFAULT;
}

static struct fs_node *fs_lookup_node(struct fs_node *node, const char *name)
{
	/* Case 1: Current node */
	if (!strcmp(name, "."))
		return node;

	/* Case 2: Parent node */
	if (!strcmp(name, ".."))
		return fs_node_parent(node);

	/*
	 * Search in the fs cache first. If there 
	 * is no entry found, read directly from
	 * the underlying device.
	 */

	/* Case 3: Cache */
	if (!fs_lookup_fast(&node, name))
		return node;

	/*
	 * No entry found in the cache, read from
	 * the underlying device next.
	 */

	/* Case 4: Read from device */
	if (!node->ops->lookup)
		return NULL;

	return node->ops->lookup(node, name);
}

static struct fs_node *fs_lookup(struct fs_node *node, const char *path)
{
	char *name, *part;

	if (!node || !path)
		return NULL;

	/*
	 * This function can be called for all nodes within
	 * the fs_node tree. The path is resolved relatively
	 * to the node passed as an argument. If the path is
	 * starting with a '/', we need to resolve the given
	 * path relatively to the root fs_node.
	 */

	if ((*path == '/') && (node != fs_root))
		node = fs_root;

	/*
	 * The strtok function is destroying the path string
	 * while we loop over all parts iteratively. Use the
	 * copy of the original string instead.
	 */

	name = bstrdup(path);
	if (!name)
		return NULL;

	part = strtok(name, "/");

	while (part) {

		/*
		 * Lookup the next node: If the given part is not
		 * found anywhere, we abort the entire procedure.
		 */

		node = fs_lookup_node(node, part);
		if (!node) {
			bfree(name);
			return NULL;
		}

		part = strtok(part, "/");
	}

	bfree(name);

	return node;
}

static struct fs_node *fs_lookup_root(const char *path)
{
	return fs_lookup(fs_root, path);
}

static int fs_readdir(struct fs_node *node __unused, struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static struct fs_node *fs_mkdir(struct fs_node *node __unused,
			     struct fs_dentry *dentry __unused)
{
	return NULL;
}

static int fs_rmdir(struct fs_node *node __unused,
		 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

/* 
 * fs_node function for files in general
 */

static int fs_read(struct fs_node *node __unused, uint64_t size __unused,
		   char *buffer __unused)
{
	return -ENOTSUP;
}

static int fs_write(struct fs_node *node __unused, uint64_t size __unused,
		    const char *buffer __unused)
{
	return -ENOTSUP;
}

static int fs_mount_device(struct device *device, const char *path)
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

		return fs_mount_device(device, path);
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

static int fs_create_root_node(struct device *device)
{
	struct fs *fs;

	list_for_each_entry(fs, &filesystems, list) {
		if (superblock_probe(device, fs))
			continue;

		fs_root = fs_node_alloc(fs, "/");
		if (!fs_root)
			return -EFAULT;

		return 0;
	}

	return -EFAULT;
}

static int fs_create_root_cache(void)
{
	node_cache = bmem_cache_create("fs_node", FS_NODE_SIZE, fs_node_init);
	if (!node_cache)
		return -ENOMEM;

	dent_cache = bmem_cache_create("fs_dent", FS_DENT_SIZE, fs_dent_init);
	if (!dent_cache)
		return -ENOMEM;

	return 0;
}

int fs_init(struct device *device)
{
	if (fs_create_root_cache())
		return -EFAULT;

	ext2_fs_init();
	isofs_fs_init();
	ramfs_fs_init();

	if (fs_create_root_node(device))
		return -EFAULT;

	return 0;
}