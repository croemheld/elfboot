#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/initcall.h>
#include <elfboot/fs.h>
#include <elfboot/file.h>
#include <elfboot/bdev.h>
#include <elfboot/cdev.h>
#include <elfboot/super.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

LIST_HEAD(filesystems);

static struct fs_node *fs_root;

/*
 * Cache for allocating filesystem nodes
 */

static struct bmem_cache *fs_node_cache;

/*
 * Debug functions
 */

#ifdef CONFIG_DEBUG_FS

static void vfs_show_node(struct tree_info *info)
{
	int indent, dlevel;
	char *symbol;

	if (!info->upper || !info->child)
		return;

	dlevel = info->level - 1;
	indent = dlevel * TREE_LINE_SIZE;

	if (!get_next_node(info->child))
		symbol = TREE_LEAF_LINE;
	else
		symbol = TREE_NODE_LINE;
	
	sprintf(info->tibuf + indent, "%s %s", symbol, info->child->name);

	if (!dlevel)
		return;

	indent -= TREE_LINE_SIZE;

	if (!get_next_node(info->upper))
		symbol = "   ";
	else
		symbol = TREE_PIPE_LINE;

	strncpy(info->tibuf + indent, symbol, TREE_LINE_SIZE);
}

static void vfs_show_tree(struct tree_info *info, struct fs_node *node)
{
	struct fs_node *curr;

	/* Enter level */
	info->level++;
	info->upper = node;
	info->child = NULL;

	for_each_node(curr, node) {
		info->child = curr;
		vfs_show_node(info);

		bprintln("VFS: %s", info->tibuf);

		if (has_sub_nodes(curr))
			continue;

		vfs_show_tree(info, curr);

		/* Leave level */
		info->level--;
		info->upper = node;
		info->child = curr;
	}
}

void vfs_dump_tree(void)
{
	struct tree_info info;

	info.level = 0;
	info.upper = NULL;
	info.child = NULL;
	info.tibuf = bmalloc(75);
	if (!info.tibuf)
		return;

	bprintln("VFS: /");

	vfs_show_tree(&info, fs_root);

	bfree(info.tibuf);
}

#endif /* CONFIG_DEBUG_FS */

/*
 * Allocating file system nodes and directory entries
 */

static void fs_node_ctor(void *obj)
{
	struct fs_node *node = obj;

	memset(node, 0, sizeof(*node));
}

struct fs_node *fs_node_alloc(const char *name)
{
	struct fs_node *node = bmem_cache_alloc(fs_node_cache);

	if (!node)
		return NULL;

	tree_node_init(&node->tree);
	strcpy(node->name, name);

	return node;
}

/*
 * File system registration for modules
 */

void fs_register(struct fs_type *fs)
{
	list_add(&fs->list, &filesystems);
}

void fs_unregister(struct fs_type *fs)
{
	list_del(&fs->list);
}

/*
 * File system functions
 */

static struct fs_type *vfs_find_type(const char *name)
{
	struct fs_type *fs;

	list_for_each_entry(fs, &filesystems, list) {
		if (strcmp(fs->name, name))
			continue;

		return fs;
	}

	return NULL;
}

/*
 * Lookup functions
 */

static int vfs_lookup_request_valid(struct fs_lookup_request *lookup_request)
{
	/* Mandatory value */
	if (!lookup_request->path)
		return -EFAULT;

	/*
	 * If no node is specified, the request must state
	 * that the lookup starts from the fs_root node.
	 */
	if (!lookup_request->node)
		return (lookup_request->flags & FS_LOOKUP_ABSOLUTE) ? 0 : -EFAULT;

	return 0;
}

static struct fs_node *vfs_lookup_tree_fast(struct fs_node *node,
	const char *name)
{
	struct fs_node *child;

	tree_for_each_child_entry(child, node, tree) {
		if (strcmp(child->name, name))
			continue;

		return child;
	}

	return NULL;
}

static struct fs_node *vfs_lookup_tree_node(struct fs_node *node,
	const char *name)
{
	/* Case 1: Current node */
	if (!strcmp(name, "."))
		return node;

	/* Case 2: Parent node */
	if (!strcmp(name, ".."))
		return fs_node_parent(node);

	/* Case 3: Cache */
	return vfs_lookup_tree_fast(node, name);
}

static struct fs_node *vfs_lookup_tree_slow(struct fs_node *node,
	const char *name)
{
	struct fs_node *nent = vfs_finddir(node, name);

	if (!nent)
		return NULL;

	tree_node_insert(&nent->tree, &node->tree);

	return nent;
}

static int vfs_lookup_tree(struct fs_lookup_request *lookup_request)
{
	struct fs_node *node;
	char *name, *part;

	/*
	 * This function is called when we request a lookup search specified
	 * by the path in the request. The lookup function modifies the path 
	 * and node members of the request, which reports how far the lookup
	 * went before an error occurred.
	 */

	if (vfs_lookup_request_valid(lookup_request))
		return -EFAULT;

	name = bstrdup(lookup_request->path);
	if (!name)
		return -ENOMEM;

	node = lookup_request->node;
	if (lookup_request->flags & FS_LOOKUP_ABSOLUTE)
		node = fs_root;

	part = strtok(name, "/");
	while (part) {
		node = vfs_lookup_tree_node(lookup_request->node, part);
		if (node)
			goto vfs_lookup_found;

		node = vfs_lookup_tree_slow(lookup_request->node, part);
		if (!node) {
			bfree(name);
			return -ENOENT;
		}

vfs_lookup_found:

		lookup_request->node = node;
		lookup_request->path = part;
		part = strtok(NULL, "/");
	}

	bfree(name);

	return 0;
}

static struct fs_node *vfs_lookup_node(struct fs_node *node, const char *path)
{
	struct fs_lookup_request lookup_request = {
		.node = node,
		.path = (char *)path
	};

	if (vfs_lookup_tree(&lookup_request))
		return NULL;

	return lookup_request.node;
}

static struct fs_node *vfs_lookup(const char *path)
{
	return vfs_lookup_node(fs_root, path);
}

/*
 * General file system functions
 */

struct fs_node *vfs_open(const char *path)
{
	return vfs_lookup(path);
}

void vfs_close(struct fs_node *node)
{

}

uint32_t vfs_read(struct fs_node *node, uint64_t offset, uint32_t length,
	void *buffer)
{
	switch (node->flags & FS_FLAGS_MASK) {
		case FS_CHARDEVICE:
			return cdev_read(node->cdev, offset, length, buffer);
		case FS_BLOCKDEVICE:
			return bdev_read(node->bdev, offset, length, buffer);
	}

	if (node->ops && node->ops->read)
		return node->ops->read(node, offset, length, buffer);

	return -ENOTSUP;
}

uint32_t vfs_write(struct fs_node *node, uint64_t offset, uint32_t length,
	const void *buffer)
{
	switch (node->flags & FS_FLAGS_MASK) {
		case FS_CHARDEVICE:
			return cdev_write(node->cdev, offset, length, buffer);
		case FS_BLOCKDEVICE:
			return bdev_write(node->bdev, offset, length, buffer);
	}

	if (node->ops && node->ops->write)
		return node->ops->write(node, offset, length, buffer);

	return -ENOTSUP;
}

struct fs_dent *vfs_readdir(struct fs_node *node, uint32_t index)
{
	return NULL;
}

struct fs_node *vfs_finddir(struct fs_node *node, const char *name)
{
	if (node->ops && node->ops->finddir)
		return node->ops->finddir(node, name);

	return NULL;
}

/*
 * Mounting filesystems
 */

static struct fs_node *vfs_mount_node(struct fs_type *fs, struct bdev *bdev,
	struct fs_node *parent, const char *name)
{
	struct fs_node *node;
	struct superblock *sb;

	if (!fs)
		return NULL;

	sb = superblock_alloc(fs, bdev);
	if (!sb)
		return NULL;

	node = fs->fill_super(sb, name);
	if (!node)
		goto mount_free_sb;

	node->sb->mount = parent;
	tree_node_insert(&node->tree, &parent->tree);

	return node;

mount_free_sb:
	bfree(sb);

	return NULL;
}

int vfs_mount_type(const char *fs_name, struct bdev *bdev, const char *path,
	const char *name)
{
	struct fs_type *fs;
	struct fs_node *node;

	if (bdev)
		bprintln("VFS: Mount device %s to %s/%s", bdev->name, path, name);

	fs = vfs_find_type(fs_name);
	if (!fs)
		return -ENOENT;

	node = vfs_lookup(path);
	if (!node)
		return -ENOENT;

	return vfs_mount_node(fs, bdev, node, name) == NULL;
}

static struct fs_node *vfs_mount_bdev(struct bdev *bdev, const char *name)
{
	struct superblock *sb;
	struct fs_node *node;
	struct fs_type *fs;

	list_for_each_entry(fs, &filesystems, list) {
		sb = superblock_alloc(fs, bdev);
		if (!sb)
			return NULL;

		node = fs->fill_super(sb, name);
		if (!node) {
			bfree(sb);
			continue;
		}

		return node;
	}

	return NULL;
}

int vfs_mount_cdev(struct cdev *cdev, const char *path, const char *name)
{
	struct fs_node *node, *nent;

	node = vfs_lookup(path);
	if (!node)
		return -ENOENT;

	nent = fs_node_alloc(name);
	if (!nent)
		return -EFAULT;

	nent->cdev = cdev;
	nent->ops  = node->ops;
	nent->flags |= FS_CHARDEVICE;

	tree_node_insert(&nent->tree, &node->tree);

	return 0;
}

int vfs_mount(struct bdev *bdev, const char *path, const char *name)
{
	struct fs_node *node, *nent;

	node = vfs_lookup(path);
	if (!node)
		return -ENOENT;

	nent = vfs_mount_bdev(bdev, name);
	if (!nent) {
		bprintln("VFS: No filesystem found for %s, abort...", bdev->name);
		return -ENOENT;
	}

	nent->flags |= FS_BLOCKDEVICE;

	tree_node_insert(&nent->tree, &node->tree);

	return 0;
}

/*
 * Initialization
 */

static int vfs_init_rootfs(struct fs_type *fs)
{
	struct superblock *sb = superblock_alloc(fs, NULL);

	if (!sb)
		return -ENOMEM;

	fs_root = fs->fill_super(sb, "[root]");
	if (!fs_root)
		goto rootfs_free_sb;

	return 0;

rootfs_free_sb:
	bfree(sb);

	return -EFAULT;
}

int vfs_init(void)
{
	struct fs_type *rootfs;

	/*
	 * Register all file system drivers here. This has to be done in order
	 * to set up the root file system so we can mount devices.
	 */
	if (vfs_modinit())
		return -EFAULT;

	/*
	 * Find the root file system driver. If this fails, we messed up big time.
	 */
	rootfs = vfs_find_type(ROOTFS);
	if (!rootfs)
		return -EFAULT;

	/*
	 * Initialize caches for allocation of filesystem nodes and directory
	 * entries. After that, allocations are done via fs_node_alloc and by
	 * fs_dent_alloc for fs_node and fs_dent structures respectively.
	 */
	fs_node_cache = bmem_cache_create(FS_NODE_CACHE, FS_NODE_SIZE, fs_node_ctor);
	if (!fs_node_cache)
		return -ENOMEM;

	/*
	 * Set up root file system. This is always "ramfs". This step should
	 * done only after all file system modules have been initialized.
	 */
	if (vfs_init_rootfs(rootfs))
		return -EFAULT;

	/*
	 * Set up the device node in our root file system. After the call is
	 * successful, we can mount any device to the /dev node.
	 */
	if (vfs_mount_type(ROOTFS, NULL, "/", "dev"))
		return -EFAULT;

	/*
	 * We also set up the TTY driver so we can print to the screen. This
	 * is done by calling another initcall macro.
	 */
	if (dev_modinit())
		return -EFAULT;

	return 0;
}