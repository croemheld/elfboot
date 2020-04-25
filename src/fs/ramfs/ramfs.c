#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/module.h>
#include <elfboot/tree.h>
#include <elfboot/printf.h>

#include <fs/ramfs.h>

static struct fs_node_ops ramfs_node_ops;

/*
 * Superblock operations to be registered
 */

static struct fs_node *ramfs_alloc_node(struct superblock *sb, const char *name)
{
	struct fs_node *node = fs_node_alloc(name);

	if (!node)
		return NULL;

	/*
	 * Assign fs_node_ops to newly allocated node
	 */
	node->ops = &ramfs_node_ops;

	return node;
}

static void ramfs_free_node(struct fs_node *node)
{

}

static struct superblock_ops ramfs_superblock_ops = {
	.alloc_node = ramfs_alloc_node,
	.free_node = ramfs_free_node
};

static struct fs_node *ramfs_fill_super(struct superblock *sb, const char *name)
{
	struct fs_node *node;

	/*
	 * The ramfs file system operates directly on memory which is why it does
	 * not have an actual device assigned to it. If there is a device present
	 * we abort this procedure.
	 */
	if (sb->bdev)
		return NULL;

	node = fs_node_alloc(name);
	if (!node)
		return NULL;

	/* Superblock */
	sb->root  = node;
	sb->mount = node;
	
	node->sb = sb;

	return node;
}

static struct fs_type fs_ramfs = {
	.name = "ramfs",
	.fill_super = ramfs_fill_super
};

/*
 * Functions for ISO9660 filesystem nodes
 */

void ramfs_open(struct fs_node *node)
{

}

void ramfs_close(struct fs_node *node)
{

}

uint32_t ramfs_read(struct fs_node *node, uint64_t off, uint32_t len, void *buf)
{
	return 0;
}

uint32_t ramfs_write(struct fs_node *node, uint64_t off, uint32_t len, void *buf)
{
	return 0;
}

struct fs_dent *ramfs_readdir(struct fs_node *node, uint32_t index)
{
	return NULL;
}

struct fs_node *ramfs_finddir(struct fs_node *node, const char *name)
{
	return NULL;
}

static struct fs_node_ops ramfs_node_ops = {
	.open = ramfs_open,
	.close = ramfs_close,
	.read = ramfs_read,
	.write = ramfs_write,
	.readdir = ramfs_readdir,
	.finddir = ramfs_finddir
};

static int ramfs_init(void)
{
	bprintln(FS_RAM ": Initialize file system module \"ramfs\"...");

	fs_register(&fs_ramfs);

	return 0;
}

static void ramfs_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(FS_RAM ": Exit module \"ramfs\"...");

	fs_unregister(&fs_ramfs);
}

module_init(ramfs_init);
module_exit(ramfs_exit);