#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

/*
 * Structure prototype for device
 */

struct device;

struct device_driver {
	const char *name;
	int (*probe)(struct device *);
	int (*open)(struct device *, const char *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
	struct list_head list;
};

struct device {
	const char *name;
	int fd;
	struct device_driver *driver;
	void *data;
};

void device_driver_register(struct device_driver *driver);

void device_driver_unregister(struct device_driver *driver);

#endif /* __ELFBOOT_DEVICE_H__ */