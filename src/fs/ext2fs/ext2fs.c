#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/module.h>
#include <elfboot/bitops.h>
#include <elfboot/math.h>
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

static struct ext2_inode *ext2fs_inode_find(struct superblock *sb,
	struct ext2_superblock *esb, uint32_t ino)
{
	struct ext2_block_group_desc bdesc;
	struct ext2_inode *inode;
	uint32_t inoidx, blkgrp;
	uint64_t offset;

	inode = bmalloc(esb->inode_size);
	if (!inode)
		return NULL;

	blkgrp = (ino - 1) / esb->inodes_per_group;
	offset = sb->block_size;
	if (sb->block_size < 2048)
		offset = sb->block_size * 2;
	offset = offset + (EXT2_BLKGRP_DESC_SIZE * blkgrp);

	if (superblock_read_offset(sb, offset, sizeof(bdesc), &bdesc))
		goto ext2_inode_error;

	inoidx = (ino - 1) % esb->inodes_per_group;
	offset = bdesc.inode_table_block * sb->block_size;
	offset = offset + (inoidx * esb->inode_size);

	if (superblock_read_offset(sb, offset, esb->inode_size, inode))
		goto ext2_inode_error;

	return inode;

ext2_inode_error:

	return NULL;
}

static uint32_t indirect_block(struct superblock *sb, uint32_t iblock,
	uint32_t cblock, uint32_t bdepth, uint32_t *buffer)
{
	uint32_t blkibn, blkmlp, blevel, blkidx;

	blkibn = sb->block_size / EXT2_MLP_LEN;

	for (blevel = bdepth; blevel + 1 > 0; blevel--) {
		blkmlp = pow(blkibn, blevel);

		if (superblock_read_block(sb, iblock, buffer))
			return 0;

		blkidx = (cblock / blkmlp) % blkibn;
		iblock = buffer[blkidx];
	}

	return iblock;
}

static uint32_t ext2fs_inode_block(struct superblock *sb,
	struct ext2_inode *inode, uint32_t offset)
{
	uint32_t cblock, blkidx, blkibn, blkmlp, i, *buffer;

	cblock = offset / sb->block_size;

	/* Direct block pointers */
	if (cblock < EXT2_DLP_NUM)
		return inode->dbp[cblock];

	buffer = bmalloc(sb->block_size);
	if (!buffer)
		return 0;

	cblock -= (EXT2_DLP_NUM - 1);
	blkibn  = sb->block_size / EXT2_MLP_LEN;

	for (i = 0, blkmlp = 1; i < 3; i++) {
		blkmlp *= pow(blkibn, i);
		cblock -= blkmlp;
		blkidx  = cblock / blkmlp;

		if (blkidx * EXT2_MLP_LEN >= sb->block_size)
			continue;

		blkidx = indirect_block(sb, inode->dbp[12 + i], cblock, i, buffer);

		break;
	}

	bfree(buffer);

	return blkidx;
}

static uint32_t ext2fs_inode_read(struct superblock *sb,
	struct ext2_inode *inode, uint64_t offset, uint32_t length, void *buffer)
{
	void *iblock;
	uint64_t sector, blknum, blkoff, sblnum;
	uint32_t nbsize, ibloff, blkidx, totlen;
	uint32_t remlen = 0, bufoff = 0;

	totlen = length;
	nbsize = sb->block_size;
	iblock = bmalloc(nbsize);
	if (!iblock)
		return -ENOMEM;

	sblnum = div(length - 1, nbsize, &remlen) + 1;
	sector = div(offset, nbsize, &ibloff);

	for (blkoff = 0; length && blkoff < sblnum; blkoff++) {
		if (blkoff)
			ibloff = 0;

		remlen = min(nbsize - ibloff, length);
		blkidx = ext2fs_inode_block(sb, inode, (blkoff + sector) * nbsize);
		if (!blkidx)
			goto ext2_inode_read_done;

		if (superblock_read_block(sb, blkidx, iblock))
			goto ext2_inode_read_done;

		memcpy(buffer + bufoff, iblock + ibloff, remlen);
		bufoff += remlen;
		length -= remlen;
	}

ext2_inode_read_done:
	bfree(iblock);

	return totlen - length;
}

static struct fs_node *ext2fs_alloc_dent(struct superblock *sb,
	uint32_t ino, const char *name)
{
	struct fs_node *fnode;
	struct ext2_inode *inode;

	fnode = ext2fs_alloc_node(sb, name);
	if (!fnode)
		return NULL;

	inode = ext2fs_inode_find(sb, sb->private, ino);
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
	uint64_t sector, secnum;
	struct fs_node *node;

	if (!sb->bdev)
		return NULL;

	esb = bmalloc(EXT2_SB_LENGTH);
	if (!esb)
		return NULL;

	sector = sb_sector(sb, 0, EXT2_SB_OFFSET);
	secnum = sb_length(sb, 0, EXT2_SB_LENGTH);

	if (bdev_read(sb->bdev, sector, secnum, esb))
		goto ext2fs_fill_free_esb;

	if (esb->magic != cputole16(EXT2_SB_MAGIC))
		goto ext2fs_fill_free_esb;

	sb->block_logs = ffs(1024) + esb->log_block_size;
	sb->block_size = 1 << sb->block_logs;

	if (esb->revision < 1)
		esb->inode_size = 128;

	sb->private = esb;

	node = ext2fs_alloc_dent(sb, EXT2_ROOT_INODE, name);
	if (!node)
		goto ext2fs_fill_free_esb;

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
	struct fs_node *dent;

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

		dent = ext2fs_alloc_dent(node->sb, dpos->inode, name);
		if (!dent)
			goto ext2fs_finddir_free_dent;

		bfree(dbeg);

		return dent;

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