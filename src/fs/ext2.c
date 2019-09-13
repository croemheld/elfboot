#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/ext2.h>

static int ext2_superblock_probe(struct device *device, struct fs *fs)
{
	return -ENOTSUP;
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