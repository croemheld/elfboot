#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/bdev.h>
#include <elfboot/bitops.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

LIST_HEAD(bdevs);

LIST_HEAD(bdev_ops);

int bdev_read(struct bdev *bdev, uint64_t sector,
	uint64_t blknum, void *buffer)
{
	if (bdev->ops && bdev->ops->read)
		return bdev->ops->read(bdev, sector, blknum, buffer);

	return -ENOTSUP;
}

int bdev_write(struct bdev *bdev, uint64_t sector,
	uint64_t blknum, const void *buffer)
{
	if (bdev->ops && bdev->ops->write)
		return bdev->ops->write(bdev, sector, blknum, buffer);

	return -ENOTSUP;
}

int bdev_ioctl(struct bdev *bdev, int request, void *args)
{
	if (bdev->ops && bdev->ops->ioctl)
		return bdev->ops->ioctl(bdev, request, args);

	return -ENOTSUP;	
}

void bdev_register_driver(struct bdev_ops *ops)
{
	list_add(&ops->list, &bdev_ops);
}

struct bdev *bdev_get(uint32_t flags, struct bdev *from)
{
	struct bdev *bdev;

	list_for_each_entry(bdev, (from) ? &from->list : &bdevs, list) {
		if (&bdev->list == &bdevs)
			return NULL;

		/*
		 * The given flags have to match exactly, which means the search
		 * is most likely faster and more accurate the more flags are in
		 * the specified parameter.
		 */
		if ((bdev->flags & flags) == flags)
			return bdev;
	}

	return NULL;
}

int bdev_init(struct bdev *bdev, struct bdev_ops *ops)
{
	struct fs_node *nent, *node;

	node = vfs_open("/dev");
	if (!node)
		return -ENOENT;

	nent = fs_node_alloc(bdev->name);
	if (!nent)
		return -ENOMEM;

	list_add(&bdev->list, &bdevs);

	nent->bdev = bdev;
	nent->flags |= FS_BLOCKDEVICE;

	/* For convenience, fill the field here */
	bdev->block_logs = ffs(bdev->block_size);
	bdev->ops = ops;

	tree_node_insert(&nent->tree, &node->tree);

	return 0;
}