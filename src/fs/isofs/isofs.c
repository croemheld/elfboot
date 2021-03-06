#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/module.h>
#include <elfboot/bitops.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/isofs.h>

#include <uapi/elfboot/common.h>

static struct fs_node_ops isofs_node_ops;

/*
 * Superblock operations to be registered
 */

static struct fs_node *isofs_alloc_node(struct superblock *sb, const char *name)
{
	struct fs_node *node = fs_node_alloc(name);

	if (!node)
		return NULL;

	/*
	 * Assign fs_node_ops to newly allocated node
	 */
	node->ops = &isofs_node_ops;
	node->sb = sb;

	return node;
}

static void isofs_free_node(struct fs_node *node)
{

}

static struct superblock_ops isofs_superblock_ops = {
	.alloc_node = isofs_alloc_node,
	.free_node = isofs_free_node
};

static struct fs_node *isofs_alloc_dent(struct superblock *sb,
	struct iso_directory_record *dent, const char *name)
{
	struct fs_node *node = isofs_alloc_node(sb, name);

	if (!node)
		return NULL;

	node->inode  = isonum_733(dent->extent);
	node->length = isonum_733(dent->size);

	/*
	 * TODO CRO: Use macros for flags
	 */
	if (*dent->flags & 0x2)
		node->flags |= FS_DIRECTORY;

	return node;
}

static struct fs_node *isofs_fill_super(struct superblock *sb, const char *name)
{
	struct iso_primary_descriptor *desc;
	struct iso_directory_record *dent;
	struct fs_node *node;

	if (!sb->bdev)
		return NULL;

	desc = bmalloc(sb->bdev->block_size);
	if (!desc)
		return NULL;

	if (bdev_read(sb->bdev, ISOFS_PRIMARY_SECTOR, 1, desc))
		goto isofs_fill_free_desc;

	if (strncmp(desc->id, ISOFS_PRIMARY_VOLUME_ID, 5))
		goto isofs_fill_free_desc;

	dent = tvptr(desc->root_directory_record);
	node = isofs_alloc_dent(sb, dent, name);
	if (!node)
		goto isofs_fill_free_desc;

	/* Root inode */
	node->sb = sb;

	/* Superblock */
	sb->root  = node;
	sb->mount = node;
	sb->block_size = isonum_723(desc->logical_block_size);
	sb->block_logs = ffs(sb->block_size);

	bfree(desc);

	return node;

isofs_fill_free_desc:
	bfree(desc);

	return NULL;
}

static struct fs_type fs_isofs = {
	.name = "isofs",
	.fill_super = isofs_fill_super
};

/*
 * Functions for ISO9660 filesystem nodes
 */

static void isofs_open(struct fs_node *node)
{

}

static void isofs_close(struct fs_node *node)
{

}

static uint32_t isofs_read(struct fs_node *node, uint64_t offset,
	uint32_t length, void *buffer)
{
	uint64_t blkoff = node->sb->block_size * node->inode + offset;

	if (superblock_read_offset(node->sb, blkoff, length, buffer))
		return -EFAULT;

	return length;
}

static uint32_t isofs_write(struct fs_node *node, uint64_t offset,
	uint32_t length, const void *buffer)
{
	return 0;
}

static struct fs_dent *isofs_readdir(struct fs_node *node, uint32_t index)
{
	return NULL;
}

static struct fs_node *isofs_finddir(struct fs_node *node, const char *name)
{
	struct iso_directory_record *dpos, *dent, *dbeg;
	uint32_t size, numblk, name_len;
	struct fs_node *nnde;

	size = node->length;
	dbeg = bmalloc(size);
	if (!dbeg)
		return NULL;

	dpos = dbeg;
	dent = dbeg;

	numblk = size >> node->sb->block_logs;
	if (superblock_read_blocks(node->sb, node->inode, numblk, dbeg))
		goto isofs_finddir_free_dent;

	while (size) {

		/*
		 * Directories covering multiple sectors: The current sector is zero-
		 * padded, the next directory entry starts at the next sector. Adjust
		 * the pointers and continue the loop.
		 */
		if (!isonum_711(dpos->length)) {
			size -= node->sb->block_size;
			dent += node->sb->block_size;
			dpos  = dent;
			continue;
		}

		/*
		 * Length of the current file name: On ISO 9660 formatted devices, an
		 * entry name can either have a terminating sequence of ";1" if it is
		 * a file or no sequence if it is a directory.
		 *
		 * The ISO9660 specification states, that every file without a dot in
		 * the name is added a trailing dot. If that is the case we also need
		 * to reduce the name length by one.
		 */
		name_len = isonum_711(dpos->name_len);
		if (!(*dpos->flags & 0x2)) {
			name_len -= 2;

			if (!strchr(name, '.') && (dpos->name[name_len - 1] == '.'))
				name_len -= 1;
		}

		/*
		 * Each file name on an ISO 9660 formatted device contains the string
		 * serminating sequence ";1", which we will leave out since comparing
		 * would fail all the time.
		 */
		if (strncmp(name, dpos->name, name_len))
			goto isofs_finddir_next;

		nnde = isofs_alloc_dent(node->sb, dpos, name);
		if (!nnde)
			goto isofs_finddir_free_dent;

		bfree(dbeg);

		return nnde;

isofs_finddir_next:
		dpos = vptradd(dpos, isonum_711(dpos->length));
	}

isofs_finddir_free_dent:
	bfree(dbeg);

	return NULL;
}

static struct fs_node_ops isofs_node_ops = {
	.open = isofs_open,
	.close = isofs_close,
	.read = isofs_read,
	.write = isofs_write,
	.readdir = isofs_readdir,
	.finddir = isofs_finddir
};

static int isofs_init(void)
{
	fs_register(&fs_isofs);

	return 0;
}

static void isofs_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(FS_ISO ": Exit module \"isofs\"...");

	fs_unregister(&fs_isofs);
}

vfs_module_init(isofs_init);
vfs_module_exit(isofs_exit);