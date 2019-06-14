#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>

/*
 * Structure prototype for device
 */

struct device;

struct device_driver {
	const char *name;
	int (*probe)(struct device *);
	int (*open)(const char *, struct device *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
	struct list_head list;
}

struct device {
	const char *name;
	int fd;
	struct device_driver *driver;
	void *data;
};

#endif /* __ELFBOOT_DEVICE_H__ */