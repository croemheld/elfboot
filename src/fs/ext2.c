#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/ext2.h>

static int ext2_read_group_desc(struct superblock *sb, uint64_t blkgrp,
				struct ext2_block_group_desc *blkdsc)
{
	int ret;
	uint64_t blkoff = blkgrp * EXT2_BLOCKS_PER_GROUP(sb);
	uint64_t grpoff = blkoff * EXT2_BLOCK_SIZE(sb);

	ret = device_read_bytes(sb->device, grpoff + EXT2_GROUP_DESC_OFFSET,
				EXT2_GROUP_DESC_LENGTH, (char *)blkdsc);
	if (ret)
		return -EFAULT;

	return 0;
}

static struct ext2_block_group_desc *ext2_group_desc(struct superblock *sb,
						     uint64_t blkgrp)
{
	struct ext2_block_group_desc *group_desc;
	int ret;

	if (blkgrp >= ext2_sb(sb)->block_group_count)
		return NULL;

	if (ext2_sb(sb)->block_groups[blkgrp])
		return ext2_sb(sb)->block_groups[blkgrp];

	group_desc = bmalloc(EXT2_GROUP_DESC_LENGTH);
	if (!group_desc)
		return NULL;
	
	ret = ext2_read_group_desc(sb, blkgrp, group_desc);
	if (ret)
		goto read_group_free;

	ext2_sb(sb)->block_groups[blkgrp] = group_desc;

read_group_free:
	if (ret)
		bfree(group_desc);

	return group_desc;
}

static int ext2_superblock_probe(struct device *device, struct fs *fs)
{
	struct ext2_superblock *esb;
	struct ext2_directory *rootp;
	struct fs_node *node;
	int ret;

	esb = bmalloc(sizeof(*esb));
	if (!esb)
		return -ENOMEM;

	ret = device_read_bytes(device, EXT2_SUPERBLOCK_OFFSET,
				EXT2_SUPERBLOCK_LENGTH, (char *)esb);
	if (ret) {
		ret = -EFAULT;
		goto sb_probe_free_esb;
	}

	if (esb->magic != cputole16(EXT2_SUPERBLOCK_MAGIC)) {
		ret = -EINVAL;
		goto sb_probe_free_esb;
	}

	if (superblock_alloc(device, fs)) {
		ret = -ENOMEM;
		goto sb_probe_free_esb;
	}

	/* Store filesystem data */
	device->sb->fs_info = esb;

	node = device->sb->root;



sb_probe_free_esb:
	if (ret)
		bfree(esb);

	return ret;
}

static int ext2_superblock_open(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static int ext2_superblock_close(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static struct superblock_ops ext2_superblock_ops = {
	.probe = ext2_superblock_probe,
	.open = ext2_superblock_open,
	.close = ext2_superblock_close,
};

static int ext2_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int ext2_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static struct fs_node *ext2_lookup(struct fs_node *node __unused,
				    const char *name __unused)
{
	return NULL;
}

static struct fs_node *ext2_mkdir(struct fs_node *node, const char *name)
{
	return NULL;
}

static int ext2_rmdir(struct fs_node *node __unused,
		 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int ext2_readdir(struct fs_node *node __unused,
			 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int ext2_read(struct fs_node *node, uint64_t size, char *buffer)
{
	return -ENOTSUP;
}

static int ext2_write(struct fs_node *node __unused, uint64_t size __unused,
		       const char *buffer __unused)
{
	return -ENOTSUP;
}

static struct fs_ops ext2_ops = {
	.open = ext2_open,
	.close = ext2_close,
	.lookup = ext2_lookup,
	.mkdir = ext2_mkdir,
	.rmdir = ext2_rmdir,
	.readdir = ext2_readdir,
	.read = ext2_read,
	.write = ext2_write,
};

static struct fs ext2_fs = {
	.name = "ext2",
	.n_ops = &ext2_ops,
	.s_ops = &ext2_superblock_ops,
	.list = LIST_HEAD_INIT(ext2_fs.list),
};

void ext2_fs_init(void)
{
	fs_register(&ext2_fs);
}