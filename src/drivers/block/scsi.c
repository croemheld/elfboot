#include <elfboot/core.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/list.h>

#include <drivers/scsi.h>

LIST_HEAD(scsi_drivers);

static int scsi_probe(struct device *device)
{
	struct device_driver *driver;

	list_for_each_entry(driver, &scsi_drivers, list) {
		if (!driver->probe)
			continue;

		if (driver->probe(device))
			continue;

		/* SCSI is a two-layered driver */
		device->driver->driver_data = driver;

		return 0;
	}

	return -EFAULT;
}

static int scsi_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int scsi_read(struct device *device __unused, uint64_t sector __unused, 
		    uint64_t size __unused, char *buffer __unused)
{
	return -ENOTSUP;
}

static int scsi_write(struct device *device __unused, uint64_t sector __unused, 
		     uint64_t size __unused, const char *buffer __unused)
{
	return -ENOTSUP;
}

static int scsi_close(struct device *device __unused)
{
	return -ENOTSUP;
}

static struct device_driver scsi_device_driver = {
	.type = DEVICE_SCSI,
	.probe = scsi_probe,
	.open = scsi_open,
	.read = scsi_read,
	.write = scsi_write,
	.close = scsi_close,
	.list = LIST_HEAD_INIT(scsi_device_driver.list),
};

void scsi_driver_register(struct device_driver *driver)
{
	list_add(&driver->list, &scsi_drivers);
}

void scsi_driver_unregister(struct device_driver *driver)
{
	list_del(&driver->list);
}

void scsi_firmware_init(void)
{
	device_driver_register(&scsi_device_driver);
}