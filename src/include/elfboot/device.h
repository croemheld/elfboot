#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

struct device;

struct device_driver {
	int (*probe)(struct device *);
	int (*open)(struct device *, const char *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
	struct list_head list;

	/*
	 * The following fields are filled
	 * with driver specific information
	 */
	
	void *driver_data;
};

struct device_io {
	uint32_t flags;

#define DEVICE_IO_FLAG_SLAVE		0x00000001
#define DEVICE_IO_FLAG_LUN		0x00000002
#define DEVICE_IO_FLAG_CHS		0x00000008
#define DEVICE_IO_FLAG_LBA		0x00000010

	/*
	 * Device communication relevant data
	 */

	uint16_t io_base;
	uint16_t control;
	uint64_t lun;

	/*
	 * Members describing the volume of the
	 * corresponding device.
	 *
	 * Wdefine members for both the CHS and
	 * the LBA addressing modes.
	 */
	
	uint32_t num_cylinders;
	uint32_t num_heads;
	uint32_t num_sectors;
	uint32_t total_sectors;

	uint32_t block_size;
	uint32_t last_block;
};

struct device {
	const char *name;
	int type;

#define DEVICE_BAD			0
#define DEVICE_BLOCK			1
#define DEVICE_CHAR			2

	/*
	 * Device flags are described by the
	 * device_flags enumerations.
	 */

	uint32_t flags;

#define DEVICE_FLAG_VIRTUAL		0x00000001

	int refcount;

	/*
	 * If this device is an actual physical
	 * device, we store the I/O ports used
	 * to communicate with the device here.
	 *
	 * For additional data belonging to this
	 * device, we provide a pointer to an
	 * arbitrary structure which is handled
	 * by the device driver.
	 */
	
	struct device_io *io;
	void *device_data;

	/*
	 * Device driver. Can be nested, i.e.
	 * consisting another driver behind.
	 */
	
	struct device_driver *driver;

	/*
	 * The devices superblock. It cointains
	 * information about size and capacity of
	 * the devices block. Usually used by the
	 * filesystem drivers.
	 */
	
	struct superblock *sb;

	struct list_head list;
};

static __always_inline bool device_io_has(struct device *device, uint32_t flag)
{
	return (device->io->flags & flag) != 0;
}

static __always_inline void device_io_set(struct device *device, uint32_t flag)
{
	device->io->flags |= flag;
}

static __always_inline bool device_is_type(struct device *device, int type)
{
	return device->type == type;
}

static __always_inline bool device_has(struct device *device, uint32_t flag)
{
	return (device->flags & flag) != 0;
}

static __always_inline void device_set(struct device *device, uint32_t flag)
{
	device->flags |= flag;
}

/*
 * This only works for physical devices since it accesses
 * the struct device_io member in the device structure.
 */

static __always_inline uint64_t device_capacity(struct device *device)
{
	return device->io->block_size * device->io->last_block;
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