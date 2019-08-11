#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/tree.h>
#include <elfboot/list.h>

#include <drivers/ata.h>
#include <drivers/scsi.h>

/*
 * List of all opened devices
 */

LIST_HEAD(devices);

/*
 * List of all available device drivers
 */

LIST_HEAD(device_drivers);

int device_probe(struct device *device)
{
	if (device->driver->probe)
		return device->driver->probe(device);

	return -ENOTSUP;
}

int device_open(struct device *device, const char *name)
{
	if (device->driver->open)
		return device->driver->open(device, name);

	return -ENOTSUP;
}

int device_read(struct device *device, uint64_t sector, 
	uint64_t size, char *buffer)
{
	if (device->driver->read)
		return device->driver->read(device, sector, size, buffer);

	return -ENOTSUP;
}

int device_read_sector(struct device *device, uint64_t sector, char *buffer)
{
	return device_read(device, sector, 1, buffer);
}

int device_write(struct device *device, uint64_t sector, 
	uint64_t size, const char *buffer)
{
	if (device->driver->write)
		return device->driver->write(device, sector, size, buffer);

	return -ENOTSUP;
}

int device_write_sector(struct device *device, uint64_t sector,
			const char *buffer)
{
	return device_write(device, sector, 1, buffer);
}

int device_close(struct device *device)
{
	if (device->driver->close)
		return device->driver->close(device);

	return -ENOTSUP;
}

int device_lookup_driver(struct device *device)
{
	struct device_driver *driver;

	list_for_each_entry(driver, &device_drivers, list) {
		if (driver->probe(device))
			continue;

		device->driver = driver;

		return 0;
	}

	return -EFAULT;
}

int device_mount(struct device *device, const char *name)
{
	device->name = bstrdup(name);
	if (!device->name)
		return -EFAULT;

	if (device_lookup_driver(device))
		return -EFAULT;

	list_add(&device->list, &devices);

	return 0;
}

int device_umount(struct device *device)
{
	if (device->refcount)
		return -EINVAL;

	bfree_const(device->name);
	device->type = 0;
	device->refcount = 0;
	device->driver = NULL;
	list_del(&device->list);

	return 0;
}

void devices_init(void)
{
	ata_firmware_init();
	scsi_firmware_init();
}

void device_driver_register(struct device_driver *driver)
{
	list_add(&driver->list, &device_drivers);
}

void device_driver_unregister(struct device_driver *driver)
{
	list_del(&driver->list);
}