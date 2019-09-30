#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

#include <fs/ext2.h>
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

struct fs_node *fs_node_alloc(struct fs_node *parent, struct fs_ops *ops,
			      const char *name)
{
	struct fs_node *child = bmem_cache_alloc(node_cache);

	if (!child)
		return NULL;

	child->ops = ops;
	strcpy(child->name, name);

	/* Add new node to children */
	fs_node_add(child, parent);

	return child;
}

struct fs_node *fs_node_create(struct fs_node *parent, const char *name)
{
	struct fs_node *node = fs_node_alloc(parent, parent->ops, name);

	/*
	 * Allocate a new node within one mount for the
	 * specified filesystem. For allocating a super
	 * node for a new mountpoint, use the dedicated
	 * function superblock_alloc_node() instead.
	 */

	if (!node)
		return NULL;

	/*
	 * We allocate within the same mount, i.e. the
	 * superblock is the same as the parents.
	 */
	node->sb = parent->sb;

	return node;
}

static void fs_dent_init(void *objp)
{
	struct fs_dentry *dentry = objp;

	/* Empty name until usage */
	memset(dentry->name, 0, 128);
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

static int fs_request_lookup_fast(struct fs_node **node, const char *name)
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

static struct fs_node *fs_request_lookup_node(struct fs_node *node,
					      const char *name)
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
	if (!fs_request_lookup_fast(&node, name))
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

static __always_inline int fs_request_valid(struct fs_lookup_req *lookup_req)
{
	/* Mandatory value */
	if (!lookup_req->path)
		return -EFAULT;

	/*
	 * If no node is specified, the request must state
	 * that the lookup starts from the fs_root node.
	 */
	if (!lookup_req->node)
		return (lookup_req->flags & FS_REQUEST_ABSOLUTE) ? 0 : -EFAULT;

	return 0;
}

static int fs_request_lookup(struct fs_lookup_req *lookup_req)
{
	struct fs_node *node;
	char *name, *part;

	/*
	 * This function is called when we request a lookup
	 * search specified by the path in the request. The
	 * function modifies both the path and node members
	 * of the request, which reports how far the lookup
	 * went before an error occurred.
	 */

	if (fs_request_valid(lookup_req))
		return -EFAULT;

	name = bstrdup(lookup_req->path);
	if (!name)
		return -ENOMEM;

	node = lookup_req->node;

	/* If requested, lookup from the root node */
	if (lookup_req->flags & FS_REQUEST_ABSOLUTE)
		node = fs_root;

	part = strtok(name, "/");

	while (part) {
		node = fs_request_lookup_node(lookup_req->node, part);
		if (!node) {
			bfree(name);
			return -ENOENT;
		}

		/* Successfully found node */
		lookup_req->node = node;
		lookup_req->path = part;

		/* Next part of path */
		part = strtok(part, "/");
	}

	bfree(name);

	return 0;
}

static struct fs_node *fs_lookup_node(struct fs_node *node, const char *path)
{
	struct fs_lookup_req lookup_req;

	init_fs_request(&lookup_req);
	lookup_req.node = node;
	lookup_req.path = (char *)path;

	if (fs_request_lookup(&lookup_req))
		return NULL;

	return lookup_req.node;
}

static struct fs_node *fs_lookup(const char *path)
{
	return fs_lookup_node(fs_root, path);
}

static int fs_readdir(struct fs_node *node __unused, struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static struct fs_node *fs_mkdir_node(struct fs_node *node, const char *path,
				     uint32_t flags __unused)
{
	char *name, *part;

	name = bstrdup(path);
	if (!name)
		return NULL;

	part = strtok(name, "/");

	while (part && node) {
		if (!node->ops->mkdir) {
			bfree(name);
			return NULL;
		}

		node = node->ops->mkdir(node, part);
		part = strtok(part, "/");
	}

	bfree(name);

	return node;
}

struct fs_node *fs_mkdir(const char *path, uint32_t flags)
{
	struct fs_lookup_req lookup_req;

	init_fs_request(&lookup_req);
	lookup_req.flags = flags;
	lookup_req.node = fs_root;
	lookup_req.path = (char *)path;

	if (!fs_request_lookup(&lookup_req))
		return lookup_req.node;

	return fs_mkdir_node(lookup_req.node, lookup_req.path, flags);
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
	struct fs_node *child, *parent;

	/*
	 * The mounting process takes the given path and creates 
	 * a new node with the device name in its directory.
	 *
	 * The mounted device is then available via:
	 * 	
	 * 	 path + "/" + device->name
	 */
	
	parent = fs_lookup(path);
	if (!parent) {
		bprintln("Could not find node for %s", path);
		return -EFAULT;
	}

	child = device->sb->root;

	/* The new node is both a mountpoint and a directory */
	fs_node_set(child, FS_NODE_MOUNT | FS_NODE_DIRECTORY);

	/* Insert the node in the VFS tree */
	fs_node_add(child, parent);

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

		fs_root = device->sb->root;
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