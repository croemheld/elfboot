#ifndef __ELFBOOT_CDEV_H__
#define __ELFBOOT_CDEV_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

/*
 * Character devices
 */

struct cdev;

struct cdev_ops {
	int (*read)(struct cdev *, uint64_t, uint64_t, void *);
	int (*write)(struct cdev *, uint64_t, uint64_t, const void *);
	int (*ioctl)(struct cdev *, int, void *);
};

struct cdev {
	const char *name;

	/*
	 * Device-specific private information. For character devices this might
	 * be the case in TTY where we might have to store additional info about
	 * the TTY buffer or current positions.
	 */
	void *private;

	/*
	 * Operations for this device. The functions can only be retrieved from a
	 * module which implements character device functions.
	 */
	struct cdev_ops *ops;

	/*
	 * List of initialized devices in the list_head structure.
	 */
	struct list_head list;
};

/*
 * Character device functions
 */

int cdev_read(struct cdev *cdev, uint64_t offset,
	uint64_t length, void *buffer);

int cdev_write(struct cdev *cdev, uint64_t offset,
	uint64_t length, const void *buffer);

int cdev_ioctl(struct cdev *cdev, int request, void *args);

int cdev_init(struct cdev *cdev, struct cdev_ops *ops);

#endif /* __ELFBOOT_CDEV_H__ */