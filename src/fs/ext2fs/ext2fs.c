#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/module.h>
#include <elfboot/bitops.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/ext2fs.h>

#include <uapi/elfboot/common.h>

static struct fs_node_ops ext2fs_node_ops;

/*
 * Superblock operations to be registered
 */

static struct fs_node *ext2fs_alloc_node(struct superblock *sb, const char *name)
{
	struct fs_node *node = fs_node_alloc(name);

	if (!node)
		return NULL;

	/*
	 * Assign fs_node_ops to newly allocated node
	 */
	node->ops = &ext2fs_node_ops;
	node->sb = sb;

	return node;
}

static void ext2fs_free_node(struct fs_node *node)
{

}

static struct superblock_ops ext2fs_superblock_ops = {
	.alloc_node = ext2fs_alloc_node,
	.free_node = ext2fs_free_node
};

/*
 * Utilities for ext2 filesystem nodes
 */

static int ext2fs_lookup_block_group(struct superblock *sb,
	struct ext2_superblock *esb, struct ext2_block_group_desc *desc,
	uint32_t ino)
{
	uint32_t blkgrp, grprem;
	uint64_t sector, blknum, offset;
	void *buffer;

	buffer = bmalloc(sb->block_size);
	if (!buffer)
		return -ENOMEM;

	blkgrp = (ino - 1) / esb->inodes_per_group;

	offset = sb->block_size;
	if (sb->block_size < 2048)
		offset = sb->block_size * 2;

	offset = offset + (EXT2_BLKGRP_DESC_SIZE * blkgrp);

	sector = sb_sector(sb, 0, offset);
	blknum = sb_length(sb, 0, sb->block_size);

	if (superblock_read_blocks(sb, sector, blknum, buffer))
		goto ext2_block_group_free_buffer;

	grprem = (EXT2_BLKGRP_DESC_SIZE * blkgrp) % sb->block_size;
	memcpy(desc, buffer + grprem, EXT2_BLKGRP_DESC_SIZE);

	bfree(buffer);

	return 0;

ext2_block_group_free_buffer:
	bfree(buffer);

	return -EFAULT;
}

static int ext2fs_lookup_inode(struct superblock *sb,
	struct ext2_superblock *esb, struct ext2_inode *inode,
	uint32_t ino)
{
	struct ext2_block_group_desc desc;
	uint32_t inoidx, blkpgr;
	uint64_t sector, blknum, offset;
	void *buffer;

	buffer = bmalloc(sb->block_size);
	if (!buffer)
		return -EFAULT;

	if (ext2fs_lookup_block_group(sb, esb, &desc, ino))
		goto ext2_inode_free_buffer;

	blkpgr = esb->blocks_per_group;
	inoidx = (ino - 1) % esb->inodes_per_group;
	offset = desc.inode_table_block * sb->block_size;
	offset = offset + (inoidx * esb->inode_size);

	sector = sb_sector(sb, 0, offset);
	blknum = sb_length(sb, 0, esb->inode_size);

	if (superblock_read_blocks(sb, sector, blknum, buffer))
		goto ext2_inode_free_buffer;

	buffer = vptradd(buffer, inoidx * esb->inode_size);
	memcpy(inode, buffer, esb->inode_size);

	bfree(buffer);

	return 0;

ext2_inode_free_buffer:
	bfree(buffer);

	return -EFAULT;
}

static struct ext2_inode *ext2fs_create_inode(struct superblock *sb,
	struct ext2_superblock *esb, uint32_t ino)
{
	struct ext2_inode *inode;

	inode = bmalloc(esb->inode_size);
	if (!inode)
		return NULL;

	if (ext2fs_lookup_inode(sb, esb, inode, ino))
		goto ext2_inode_free;

	dprintf(FS_EXT2 ": Inode type: %04x\n", inode->type);

	return inode;

ext2_inode_free:
	bfree(inode);

	return NULL;
}

static uint32_t ext2fs_inode_read(struct superblock *sb,
	struct ext2_inode *inode, uint64_t offset, uint32_t length, void *buffer)
{
	void *iblock;
	uint64_t sector, blknum, blkoff, sblnum;
	uint32_t nbsize, ibloff, blkidx, cblock, totlen;
	uint32_t remlen = 0, bufoff = 0;

	totlen = length;
	nbsize = sb->block_size;
	iblock = bmalloc(nbsize);
	if (!iblock)
		return -ENOMEM;

	sblnum = ((length - 1) / nbsize) + 1;

	for (blkoff = 0; length && blkoff < sblnum; blkoff++) {
		ibloff = blkoff ? 0 : offset % nbsize;
		remlen = min(nbsize - ibloff, length);
		cblock = (blkoff * remlen) / sb->block_size;

		/*
		 * TODO CRO: Support for singly, doubly and trebly linked blocks
		 */

		if (cblock >= EXT2_DLP_NUM)
			goto ext2_inode_read_done;

		blkidx = inode->dbp[cblock];
		sector = sb_sector(sb, 0, blkidx * nbsize);
		blknum = sb_length(sb, ibloff, remlen);

		if (superblock_read_blocks(sb, sector, blknum, iblock))
			goto ext2_inode_read_done;

		memcpy(buffer + bufoff, iblock + ibloff, remlen);
		bufoff += remlen;
		length -= remlen;
	}

ext2_inode_read_done:
	bfree(iblock);

	return totlen - length;
}

static struct ext2_directory *ext2fs_lookup_edent(struct superblock *sb,
	struct ext2_inode *inode, const char *name)
{
	/*
	char *part;
	struct ext2_directory *dent;
	uint32_t rlen, size = 0;

	dent = bmalloc(sizeof(*dent));
	if (!dent)
		return NULL;

	while (size < inode->size_lower) {
		rlen = sizeof(*dent);

		if (ext2fs_inode_read(sb, inode, size, rlen, dent))
			goto ext2_lookup_edent_fail;

		part = bmalloc(dent->name_length);
		if (!part)
			goto ext2_lookup_edent_fail;

		rlen = dent->name_length;
		if (ext2fs_inode_read(sb, inode, size + sizeof(*dent), rlen, part))
			goto ext2_lookup_ename_fail;

		if (!strncmp(name, part, rlen)) {
			bfree(part);

			return dent;
		}

		size += dent->size;
	}

ext2_lookup_ename_fail:
	bfree(part);

ext2_lookup_edent_fail:
	bfree(dent);
*/
	return NULL;
}

static struct fs_node *ext2fs_alloc_dent(struct superblock *sb,
	uint32_t ino, const char *name)
{
	struct fs_node *fnode;
	struct ext2_inode *inode;

	fnode = ext2fs_alloc_node(sb, name);
	if (!fnode)
		return NULL;

	inode = ext2fs_create_inode(sb, sb->private, ino);
	if (!inode)
		goto ext2fs_alloc_free_node;

	fnode->inode  = ino;
	fnode->length = inode->size_lower;
	fnode->private = inode;

	return fnode;

ext2fs_alloc_free_node:
	bfree(fnode);

	return NULL;
}

static struct fs_node *ext2fs_fill_super(struct superblock *sb, const char *name)
{
	struct ext2_superblock *esb;
	struct ext2_inode *rino;
	struct fs_node *node;
	uint64_t sector, blknum;

	if (!sb->bdev)
		return NULL;

	esb = bmalloc(EXT2_SUPERBLOCK_LENGTH);
	if (!esb)
		return NULL;

	sector = sb_sector(sb, 0, EXT2_SUPERBLOCK_OFFSET);
	blknum = sb_length(sb, 0, EXT2_SUPERBLOCK_LENGTH);

	if (superblock_read_blocks(sb, sector, blknum, esb))
		goto ext2fs_fill_free_esb;

	if (esb->magic != cputole16(EXT2_SUPERBLOCK_MAGIC))
		goto ext2fs_fill_free_esb;

	sb->block_logs = esb->log_block_size;
	sb->block_size = 1024 << sb->block_logs;

	if (esb->revision < 1)
		esb->inode_size = 128;

	sb->private = esb;

	node = ext2fs_alloc_dent(sb, EXT2_ROOT_INODE, name);
	if (!node)
		goto ext2fs_fill_free_esb;

	rino = node->private;

	if (rino->type & EXT2_TYPE_DIR)
		dprintf(FS_EXT2 ": Root inode is confirmed to be a directory!\n");

	/* Root inode */
	node->sb = sb;

	/* Superblock */
	sb->root  = node;
	sb->mount = node;

	return node;

ext2fs_fill_free_esb:
	bfree(esb);

	return NULL;
}

static struct fs_type fs_ext2fs = {
	.name = "ext2fs",
	.fill_super = ext2fs_fill_super
};

/*
 * Functions for ext2 filesystem nodes
 */

static void ext2fs_open(struct fs_node *node)
{

}

static void ext2fs_close(struct fs_node *node)
{

}

static uint32_t ext2fs_read(struct fs_node *node, uint64_t offset,
	uint32_t length, void *buffer)
{
	return ext2fs_inode_read(node->sb, node->private, offset, length, buffer);
}

static uint32_t ext2fs_write(struct fs_node *node, uint64_t offset,
	uint32_t length, const void *buffer)
{
	return 0;
}

static struct fs_dent *ext2fs_readdir(struct fs_node *node, uint32_t index)
{
	return NULL;
}

static struct fs_node *ext2fs_finddir(struct fs_node *node, const char *name)
{
	struct ext2_inode *inode;
	struct ext2_directory *dpos, *dbeg;
	uint32_t size;

	inode = node->private;

	size = inode->size_lower;
	dbeg = bmalloc(size);
	if (!dbeg)
		return NULL;

	dpos = dbeg;

	if (ext2fs_inode_read(node->sb, inode, 0, size, dbeg) != size)
		goto ext2fs_finddir_free_dent;

	while (size) {
		if (!dpos->size)
			break;

		if (strncmp(name, dpos->name, dpos->name_length))
			goto ext2fs_finddir_next;

		bfree(dbeg);

		return ext2fs_alloc_dent(node->sb, dpos->inode, name);

ext2fs_finddir_next:
		dpos = vptradd(dpos, dpos->size);
	}

ext2fs_finddir_free_dent:
	bfree(dbeg);

	return NULL;
}

static struct fs_node_ops ext2fs_node_ops = {
	.open = ext2fs_open,
	.close = ext2fs_close,
	.read = ext2fs_read,
	.write = ext2fs_write,
	.readdir = ext2fs_readdir,
	.finddir = ext2fs_finddir
};

static int ext2fs_init(void)
{
	fs_register(&fs_ext2fs);

	return 0;
}

static void ext2fs_exit(void)
{
	/*
	 * Not supported.
	 */

	bprintln(FS_EXT2 ": Exit module \"ext2fs\"...");

	fs_unregister(&fs_ext2fs);
}

vfs_module_init(ext2fs_init);
vfs_module_exit(ext2fs_exit);