#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

static int memdisk_probe(struct device *device)
{
	if (!device_is_interface(device, DEVICE_INTERFACE_MEMDISK))
		return -EFAULT;

	return 0;
}

static int memdisk_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int memdisk_read(struct device *device, uint64_t sector, uint64_t size,
			char *buffer)
{
	// memcpy(buffer, device->)
	
	return -ENOTSUP;
}

static int memdisk_write(struct device *device, uint64_t sector, uint64_t size,
			 const char *buffer)
{
	return -ENOTSUP;
}

static int memdisk_close(struct device *device __unused)
{
	return -ENOTSUP;
}

static struct device_driver memdisk_device_driver = {
	.probe = memdisk_probe,
	.open = memdisk_open,
	.read = memdisk_read,
	.write = memdisk_write,
	.close = memdisk_close,
	.list = LIST_HEAD_INIT(memdisk_device_driver.list),
};

void memdisk_firmware_init(void)
{
	device_driver_register(&memdisk_device_driver);
}