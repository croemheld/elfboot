#include <elfboot/core.h>
#include <elfboot/cdev.h>
#include <elfboot/list.h>

LIST_HEAD(cdevs);

int cdev_init(struct cdev *cdev, struct cdev_ops *ops)
{
	return 0;
}

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