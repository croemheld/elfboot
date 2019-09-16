#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/ramfs.h>

static int ramfs_superblock_probe(struct device *device, struct fs *fs)
{
	struct fs_node *node;

	if (device->info.interface != DEVICE_INTERFACE_RAMDISK)
		return -EFAULT;

	if (superblock_alloc(device, fs))
		return -EFAULT;

	node = device->sb->root;

	node->sb = device->sb;

	/* No initial offsets */
	node->offset = vptrtuint(device->device_data) / RAMFS_BLOCK_SIZE;
	node->size   = 0;

	return 0;
}

static int ramfs_superblock_open(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static int ramfs_superblock_close(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static struct superblock_ops ramfs_superblock_ops = {
	.probe = ramfs_superblock_probe,
	.open = ramfs_superblock_open,
	.close = ramfs_superblock_close,
};

static int ramfs_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int ramfs_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static struct fs_node *ramfs_lookup(struct fs_node *node __unused,
				    const char *name __unused)
{
	return NULL;
}

static struct fs_node *ramfs_mkdir(struct fs_node *node, const char *name)
{
	return NULL;
}

static int ramfs_rmdir(struct fs_node *node __unused,
		 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int ramfs_readdir(struct fs_node *node __unused,
			 struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}

static int ramfs_read(struct fs_node *node, uint64_t size, char *buffer)
{
	uint64_t blocks;
	char *tmpbuf;
	int ret = 0;

	tmpbuf = bmalloc(size);
	if (!tmpbuf)
		return -ENOMEM;

	blocks = calculate_blocks(node, size);

	if (device_read(node->sb->device, node->offset, blocks, tmpbuf)) {
		ret = -EFAULT;
		goto ramfs_read_free_tmpbuf;
	}

	/* Copy the content into the actual buffer */
	memcpy(buffer, tmpbuf, size);

ramfs_read_free_tmpbuf:
	bfree(tmpbuf);

	return ret;
}

static int ramfs_write(struct fs_node *node __unused, uint64_t size __unused,
		       const char *buffer __unused)
{
	return -ENOTSUP;
}

static struct fs_ops ramfs_ops = {
	.open = ramfs_open,
	.close = ramfs_close,
	.lookup = ramfs_lookup,
	.mkdir = ramfs_mkdir,
	.rmdir = ramfs_rmdir,
	.readdir = ramfs_readdir,
	.read = ramfs_read,
	.write = ramfs_write,
};

static struct fs ramfs_fs = {
	.name = "ramfs",
	.n_ops = &ramfs_ops,
	.s_ops = &ramfs_superblock_ops,
	.list = LIST_HEAD_INIT(ramfs_fs.list),
};

void ramfs_fs_init(void)
{
	fs_register(&ramfs_fs);
}