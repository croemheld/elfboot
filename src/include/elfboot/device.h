#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

enum device_flags {
	DEVICE_FLAGS_VIRTUAL,		/* Not a physical device */
	DEVICE_FLAGS_SLAVE,		/* 0 = master, 1 = slave */
	DEVICE_FLAGS_LBA,		/* 0 = only CHS, 1 = LBA */
	DEVICE_FLAGS_CHS_VALID,		/* CHS values are usable */
	DEVICE_FLAGS_IO_ADDRESS,	/* Device has IO address */
};

/*
 * Structure prototype for device
 */

struct device;

struct device_params {
	uint16_t flags;

	/* CHS values */
	uint32_t num_cylinders;
	uint32_t num_heads;
	uint32_t num_sectors;
	uint32_t total_sectors;

	/* Sector size */
	uint32_t sector_size;

	/* IO address */
	uint16_t io_base;
	uint16_t control;
};

struct device_driver {
	int type;
	int (*probe)(struct device *);
	int (*open)(struct device *, const char *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
	struct list_head list;

	/*
	 * The following fields are filled
	 * with device specific information
	 */
	
	void *driver_data;
};

struct device {
	const char *name;
	int type;

#define DEVICE_BAD			0
#define DEVICE_ATA			1
#define DEVICE_ATAPI			2
#define DEVICE_SCSI			3
#define DEVICE_TTY			3

	int refcount;
	struct list_head list;
	struct device_params params;
	struct device_driver *driver;

	/*
	 * The following fields are filled
	 * with device specific information
	 */
	
	void *device_data;
};

static inline int device_is_type(struct device *device, int type)
{
	return device->type == type;
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

int device_lookup_driver(struct device *device);

int device_mount(struct device *device, const char *name);

int device_umount(struct device *device);

void devices_init(void);

void device_driver_register(struct device_driver *driver);

void device_driver_unregister(struct device_driver *driver);

#endif /* __ELFBOOT_DEVICE_H__ */