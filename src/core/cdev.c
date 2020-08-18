#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/cdev.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

LIST_HEAD(cdevs);

int cdev_read(struct cdev *cdev, uint64_t offset,
	uint64_t length, void *buffer)
{
	if (cdev->ops && cdev->ops->read)
		return cdev->ops->read(cdev, offset, length, buffer);

	return -ENOTSUP;
}

int cdev_write(struct cdev *cdev, uint64_t offset,
	uint64_t length, const void *buffer)
{
	if (cdev->ops && cdev->ops->write)
		return cdev->ops->write(cdev, offset, length, buffer);

	return -ENOTSUP;
}

int cdev_init(struct cdev *cdev, struct cdev_ops *ops)
{
	cdev->ops = ops;

	/* Directly mount device to "/dev" node */
	if (vfs_mount_cdev(cdev, "/dev", cdev->name))
		return -EFAULT;

	list_add(&cdev->list, &cdevs);

	return 0;
}