#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

enum device_type {
	DEVICE_BAD,
	DEVICE_ATA,
	DEVICE_ATAPI,
	DEVICE_TTY,
};

enum device_flags {
	DEVICE_FLAGS_SLAVE,		/* 0 = master, 1 = slave */
	DEVICE_FLAGS_LBA,		/* 0 = only CHS, 1 = LBA */
};

/*
 * Structure prototype for device
 */

struct device;

struct device_params {
	int type;
	uint16_t io_base;
	uint16_t control;
	uint16_t flags;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t spt;
	uint64_t total_sectors;
	uint16_t bps;
};

struct device_driver {
	int type;
	int (*probe)(struct device *);
	int (*open)(struct device *, const char *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
	struct list_head list;
};

struct device {
	int fd;
	const char *name;
	struct device_params params;
	struct device_driver *driver;
	void *data;
};

static inline int device_is_type(struct device *device, int type)
{
	return device->params.type == type;
}

static inline int device_has_flag(struct device *device, int flag)
{
	return device->params.flags & _BITUL(flag);
}

static inline int device_set_flag(struct device *device, int flag)
{
	return device->params.flags |= _BITUL(flag);
}

int device_probe(struct device *device);

int device_open(struct device *device, const char *name);

int device_read(struct device *device, uint64_t sector, 
	uint64_t size, char *buffer);

int device_write(struct device *device, uint64_t sector, 
	uint64_t size, const char *buffer);

int device_close(struct device *device);

int device_install_firmware(struct device *device);

void devices_init(void);

void device_driver_register(struct device_driver *driver);

void device_driver_unregister(struct device_driver *driver);

#endif /* __ELFBOOT_DEVICE_H__ */