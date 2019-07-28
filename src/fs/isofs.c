#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/vfs.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

static int isofs_superblock_probe(struct device *device)
{
	return -ENOTSUP;
}

static int isofs_superblock_open(struct fs_superblock *sb __unused)
{
	return -ENOTSUP;
}

static int isofs_superblock_close(struct fs_superblock *sb __unused)
{
	return -ENOTSUP;
}

static struct fs_node *isofs_superblock_alloc_node(struct fs_superblock *sb __unused)
{
	return NULL;
}

static void isofs_superblock_free_node(struct fs_node *node __unused)
{
	
}

static struct superblock_ops isofs_superblock_ops = {
	.probe = isofs_superblock_probe,
	.open = isofs_superblock_open,
	.close = isofs_superblock_close,
	.alloc_node = isofs_superblock_alloc_node,
};

static int isofs_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int isofs_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static struct fs_node *isofs_lookup(struct fs_node *node __unused,
				    const char *name __unused)
{
	return NULL;
}

static int isofs_readdir(struct fs_node *node __unused,
			 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int isofs_read(struct fs_node *node __unused, uint64_t offset __unused,
		      char *buffer __unused)
{
	return -ENOTSUP;
}

static int isofs_write(struct fs_node *node __unused, uint64_t offset __unused,
		       const char *buffer __unused)
{
	return -ENOTSUP;
}

static struct fs_ops isofs_ops = {
	.open = isofs_open,
	.close = isofs_close,
	.lookup = isofs_lookup,
	.readdir = isofs_readdir,
	.read = isofs_read,
	.write = isofs_write,
};

static struct fs isofs_fs = {
	.name = "isofs",
	.nops = &isofs_ops,
	.sops = &isofs_superblock_ops,
	.list = LIST_HEAD_INIT(isofs_fs.list),
}

void isofs_fs_init(void)
{
	vfs_register_fs(&isofs_fs);
}