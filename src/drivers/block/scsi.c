#include <elfboot/core.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

#include <drivers/scsi.h>

LIST_HEAD(scsi_drivers);

static struct scsi_driver *scsi_get_driver(struct device *device)
{
	if (!device->driver->driver_data)
		return NULL;

	return device->driver->driver_data;
}

static int scsi_request_sense(struct device *device)
{
	struct scsi_request_sense rs;
	struct scsi_request_sense_data rsd;
	struct scsi_driver *driver;
	int r;

	rs.cmd = SCSI_CMD_REQUEST_SENSE;
	rs.lun = device->params.lun << SCSI_LUN_SHIFT;
	rs._reserved1 = 0;
	rs._reserved2 = 0;
	rs.len = 0x12;
	rs.control = 0;
	memset(rs.pad, 0, sizeof(rs.pad));

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	r = driver->read(device, (char *)&rs, sizeof(rs),
			 (char *)&rsd, sizeof(rsd));

	if (r)
		return r;

	return 0;
}

static int scsi_inquiry(struct device *device)
{
	struct scsi_inquiry iq;
	struct scsi_inquiry_data iqd;
	struct scsi_driver *driver;
	int r;

	iq.cmd = SCSI_CMD_INQUIRY;
	iq.lun = device->params.lun << SCSI_LUN_SHIFT;
	iq.page = 0;
	iq._reserved = 0;
	iq.len = 0x24;
	iq.control = 0;
	memset(iq.pad, 0, sizeof(iq.pad));

	/* Get the actual driver for this device */
	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	r = driver->read(device, (char *)&iq, sizeof(iq), 
			 (char *)&iqd, sizeof(iqd));

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	/*
	 * Device type in iqd
	 */

	bprintln("SCSI: Vendor = %.*s", sizeof(iqd.vendor), iqd.vendor);
	bprintln("SCSI: Prodid = %.*s", sizeof(iqd.prodid), iqd.prodid);
	bprintln("SCSI: Prodrv = %.*s", sizeof(iqd.prodrev), iqd.prodrev);

	return 0;
}

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

		scsi_inquiry(device);

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

void scsi_driver_register(struct scsi_driver *driver)
{
	list_add(&driver->list, &scsi_drivers);
}

void scsi_driver_unregister(struct scsi_driver *driver)
{
	list_del(&driver->list);
}

void scsi_firmware_init(void)
{
	device_driver_register(&scsi_device_driver);
}