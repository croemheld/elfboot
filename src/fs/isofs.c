#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <fs/isofs.h>

static int isofs_superblock_probe(struct device *device, struct fs *fs)
{
	struct iso_primary_descriptor *pvd;
	struct iso_directory_record *rootp;
	struct fs_node *node;
	int ret;

	pvd = bmalloc(device->info.block_size);
	if (!pvd)
		return -ENOMEM;

	/* Read the primary volume descriptor into the buffer */
	ret = device_read_sector(device, ISOFS_PRIMARY_SECTOR, (char *)pvd);
	if (ret) {
		ret = -EFAULT;
		goto sb_probe_free_pvd;
	}

	/* Is this indeed an ISO 9660 formatted device? */
	if (strncmp(pvd->id, ISOFS_PRIMARY_VOLUME_ID, 5)) {
		ret = -EFAULT;
		goto sb_probe_free_pvd;
	}

	/*
	 * Superblock allocation
	 */

	if (superblock_alloc(device, fs)) {
		ret = -ENOMEM;
		goto sb_probe_free_pvd;
	}

	/* Store filesystem data */
	device->sb->fs_info = pvd;

	node = device->sb->root;
	rootp = isofs_root_dir(pvd);

	node->sb = device->sb;
	node->offset = isonum_733(rootp->extent);
	node->size   = isonum_711(rootp->length);

	bprintln("Device %s root directory at sector %llx (%lu bytes)",
		 device->name, node->offset, node->size);

	/* Set block informations */
	device->sb->last_block = isonum_733(pvd->volume_space_size);
	device->sb->block_size = isonum_723(pvd->logical_block_size);

sb_probe_free_pvd:
	if (ret)
		bfree(pvd);

	return ret;
}

static int isofs_superblock_open(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static int isofs_superblock_close(struct superblock *sb __unused)
{
	return -ENOTSUP;
}

static struct superblock_ops isofs_superblock_ops = {
	.probe = isofs_superblock_probe,
	.open = isofs_superblock_open,
	.close = isofs_superblock_close,
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

static int isofs_read(struct fs_node *node __unused, uint64_t size __unused,
		      char *buffer __unused)
{
	return -ENOTSUP;
}

static int isofs_write(struct fs_node *node __unused, uint64_t size __unused,
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
	.n_ops = &isofs_ops,
	.s_ops = &isofs_superblock_ops,
	.list = LIST_HEAD_INIT(isofs_fs.list),
};

void isofs_fs_init(void)
{
	fs_register(&isofs_fs);
}