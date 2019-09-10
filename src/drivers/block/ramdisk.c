#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

#define DEVICE_RAMDISK_SECTOR_SIZE	GENERIC_DEVICE_SECTOR_SIZE

static int ramdisk_probe(struct device *device)
{
	if (!device_is_interface(device, DEVICE_INTERFACE_RAMDISK))
		return -EFAULT;

	return 0;
}

static int ramdisk_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int ramdisk_read(struct device *device, uint64_t sector, uint64_t size,
			char *buffer)
{
	/*
	 * A ramdisk device stores the beginning of the memory region it 
	 * is managing in the device_data member of the device structure.
	 */

	memcpy(buffer, 
	       device->device_data + (sector * DEVICE_RAMDISK_SECTOR_SIZE),
	       size * DEVICE_RAMDISK_SECTOR_SIZE);

	return 0;
}
	
static int ramdisk_write(struct device *device, uint64_t sector, uint64_t size,
			 const char *buffer)
{
	memcpy(device->device_data + (sector * DEVICE_RAMDISK_SECTOR_SIZE),
	       buffer,
	       size * DEVICE_RAMDISK_SECTOR_SIZE);

	return 0;
}

static int ramdisk_close(struct device *device __unused)
{
	return -ENOTSUP;
}

static struct device_driver ramdisk_device_driver = {
	.probe = ramdisk_probe,
	.open = ramdisk_open,
	.read = ramdisk_read,
	.write = ramdisk_write,
	.close = ramdisk_close,
	.list = LIST_HEAD_INIT(ramdisk_device_driver.list),
};

void ramdisk_firmware_init(void)
{
	device_driver_register(&ramdisk_device_driver);
}