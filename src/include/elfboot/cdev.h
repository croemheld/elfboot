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
};

struct cdev {
	struct cdev_ops *ops;
};

/*
 * Character device functions
 */

int cdev_init(struct cdev *cdev, struct cdev_ops *ops);

int cdev_read(struct cdev *cdev, uint64_t offset,
	uint64_t length, void *buf);

int cdev_write(struct cdev *cdev, uint64_t offset,
	uint64_t length, const void *buf);

#endif /* __ELFBOOT_CDEV_H__ */