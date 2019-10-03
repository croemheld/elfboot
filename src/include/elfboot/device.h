#ifndef __ELFBOOT_DEVICE_H__
#define __ELFBOOT_DEVICE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

#define GENERIC_DEVICE_SECTOR_SHIFT	9
#define GENERIC_DEVICE_SECTOR_SIZE	_BITUL(GENERIC_DEVICE_SECTOR_SHIFT)

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

struct device_io {

	/*
	 * Device communication relevant data
	 */

	uint16_t io_base;
	uint16_t control;
};

struct device_info {
	uint32_t flags;

#define DEVICE_FLAG_VIRTUAL		0x00000001
#define DEVICE_FLAG_SLAVE		0x00000002
#define DEVICE_FLAG_LUN			0x00000004
#define DEVICE_FLAG_CHS			0x00000008
#define DEVICE_FLAG_LBA			0x00000010

	int interface;

#define DEVICE_INTERFACE_UNKNOWN	0
#define DEVICE_INTERFACE_ATA		1
#define DEVICE_INTERFACE_ATAPI		2
#define DEVICE_INTERFACE_SCSI		3
#define DEVICE_INTERFACE_RAMDISK	4

	uint64_t lun;

	union {
		/* CHS */
		struct {

			uint32_t cylinders;
			uint32_t heads;
			uint32_t sectors;
			uint32_t total_sectors;
		};

		/* LBA */
		struct {
			uint64_t block_size;
			uint64_t last_block;
		};
	};

	/* Device specific */

	uint8_t type;
	uint8_t removable;
};

struct device {
	const char *name;
	int type;

#define DEVICE_BAD			0
#define DEVICE_BLOCK			1
#define DEVICE_CHAR			2

	int refcount;
	struct device_info info;

	/*
	 * If this device is an actual physical
	 * device, we store the I/O ports used
	 * to communicate with the device here.
	 */
	
	struct device_io *io;

	/*
	 * Device driver. Can be nested, i.e.
	 * consisting another driver behind.
	 */
	
	struct device_driver *driver;
	void *device_data;

	/*
	 * The devices superblock. It cointains
	 * information about size and capacity of
	 * the devices block. Usually used by the
	 * filesystem drivers.
	 */
	
	struct superblock *sb;
	struct list_head list;
};

static __always_inline bool device_is_type(struct device *device, int type)
{
	return device->type == type;
}

static __always_inline bool device_has(struct device *device, uint32_t flag)
{
	return (device->info.flags & flag) != 0;
}

static __always_inline void device_set(struct device *device, uint32_t flag)
{
	device->info.flags |= flag;
}

static __always_inline void device_get(struct device *device)
{
	device->refcount++;
}

static __always_inline int device_put(struct device *device)
{
	if (!(device->refcount > 0)) {
		bprintln("Device %s is already unused!", device->name);
		return -EFAULT;
	}

	--device->refcount;

	return 0;
}

static __always_inline bool device_is_interface(struct device *device,
						int interface)
{
	return device->info.interface == interface;
}

/*
 * This only works for physical devices since it accesses
 * the struct device_io member in the device structure.
 */

static __always_inline uint64_t device_capacity(struct device *device)
{
	return device->info.block_size * device->info.last_block;
}

int device_probe(struct device *device);

int device_open(struct device *device, const char *name);

int device_read(struct device *device, uint64_t sector, 
		uint64_t size, char *buffer);

static __always_inline int device_read_sector(struct device *device,
					      uint64_t sector, char *buffer)
{
	return device_read(device, sector, 1, buffer);
}

int device_read_bytes(struct device *device, uint64_t offset,
		      uint64_t length, char *buffer);

int device_write(struct device *device, uint64_t sector, 
		 uint64_t size, const char *buffer);

static __always_inline int device_write_sector(struct device *device,
					       uint64_t sector,
					       const char *buffer)
{
	return device_write(device, sector, 1, buffer);
}

int device_close(struct device *device);

int device_create(struct device *device, const char *name);

int device_destroy(struct device *device);

void devices_init(void);

void device_driver_register(struct device_driver *driver);

void device_driver_unregister(struct device_driver *driver);

#endif /* __ELFBOOT_DEVICE_H__ */