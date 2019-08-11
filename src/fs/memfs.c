#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

static int memfs_superblock_probe(struct device *device __unused,
				  struct fs *fs __unused)
{
	return -ENOTSUP;
}

static int memfs_superblock_open(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static int memfs_superblock_close(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static struct superblock_ops memfs_superblock_ops = {
	.probe = memfs_superblock_probe,
	.open = memfs_superblock_open,
	.close = memfs_superblock_close,
};

static int memfs_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int memfs_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static struct fs_node *memfs_lookup(struct fs_node *node __unused,
				    const char *name __unused)
{
	return NULL;
}

static int memfs_readdir(struct fs_node *node __unused,
			 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int memfs_read(struct fs_node *node __unused, uint64_t offset __unused,
		      char *buffer __unused)
{
	return -ENOTSUP;
}

static int memfs_write(struct fs_node *node __unused, uint64_t offset __unused,
		       const char *buffer __unused)
{
	return -ENOTSUP;
}

static struct fs_ops memfs_ops = {
	.open = memfs_open,
	.close = memfs_close,
	.lookup = memfs_lookup,
	.readdir = memfs_readdir,
	.read = memfs_read,
	.write = memfs_write,
};

static struct fs memfs_fs = {
	.name = "memfs",
	.n_ops = &memfs_ops,
	.s_ops = &memfs_superblock_ops,
	.list = LIST_HEAD_INIT(memfs_fs.list),
};

void memfs_fs_init(void)
{
	fs_register(&memfs_fs);
}