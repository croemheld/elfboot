#include <elfboot/core.h>
#include <elfboot/device.h>
#include <elfboot/list.h>

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

int device_write(struct device *device, uint64_t sector, 
	uint64_t size, const char *buffer)
{
	if (device->driver->write)
		return device->driver->write(device, sector, size, buffer);

	return -ENOTSUP;
}

int device_close(struct device *device)
{
	if (device->driver->close)
		return device->driver->close(device);

	return -ENOTSUP;
}

void device_driver_register(struct device_driver *driver)
{
	list_add(&driver->list, &device_drivers);
}

void device_driver_unregister(struct device_driver *driver)
{
	list_del(&driver->list);
}