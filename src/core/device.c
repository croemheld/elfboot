#include <elfboot/core.h>
#include <elfboot/device.h>

/*
 * List of all mounted devices
 */

LIST_HEAD(devices);

int device_probe(struct device *device)
{
	if (device->driver->probe)
		return device->driver->probe(name, device);

	return -ENOTSUP;
}

int device_open(const char *name, struct device *device)
{
	if (device->driver->open)
		return device->driver->open(name, device);

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

