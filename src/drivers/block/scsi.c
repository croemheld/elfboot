#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

#include <drivers/scsi.h>

LIST_HEAD(scsi_drivers);

static inline struct scsi_driver *scsi_get_driver(struct device *device)
{
	return device->device_data;
}

static int scsi_request_sense(struct device *device)
{
	struct scsi_request_sense rs;
	struct scsi_request_sense_data rsd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rs.cmd = SCSI_CMD_REQUEST_SENSE;
	rs.lun = device->info.lun << SCSI_LUN_SHIFT;
	rs._reserved1 = 0;
	rs._reserved2 = 0;
	rs.len = 0x12;
	rs.control = 0;
	memset(rs.pad, 0, sizeof(rs.pad));

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

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	iq.cmd = SCSI_CMD_INQUIRY;
	iq.lun = device->info.lun << SCSI_LUN_SHIFT;
	iq.page = 0;
	iq._reserved = 0;
	iq.len = 0x24;
	iq.control = 0;
	memset(iq.pad, 0, sizeof(iq.pad));

	r = driver->read(device, (char *)&iq, sizeof(iq), 
			 (char *)&iqd, sizeof(iqd));

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	bprintln("SCSI: Vendor = %.*s", sizeof(iqd.vendor), iqd.vendor);
	bprintln("SCSI: Prodid = %.*s", sizeof(iqd.prodid), iqd.prodid);
	bprintln("SCSI: Prodrv = %.*s", sizeof(iqd.prodrev), iqd.prodrev);

	device->info.type = iqd.type & SCSI_DEVICE_TYPE_MASK;
	device->info.removable = iqd.rmb >> SCSI_RMB_SHIFT;

	return 0;
}

static int scsi_read_capacity10(struct device *device)
{
	struct scsi_read_capacity10 rc;
	struct scsi_read_capacity10_data rcd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rc.cmd = SCSI_CMD_READ_CAPACITY10;
	rc.lun = device->info.lun << SCSI_LUN_SHIFT;
	rc.lba = 0;
	rc._reserved1 = 0;
	rc._reserved2 = 0;
	rc.pmi = 0;
	rc.control = 0;
	rc.pad = 0;

	r = driver->read(device, (char *)&rc, sizeof(rc),
			 (char *)&rcd, sizeof(rcd));

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	device->info.last_block = betocpu32(rcd.last_block);
	device->info.block_size = betocpu32(rcd.block_size);

	return 0;
}

static int scsi_read_capacity16(struct device *device)
{
	struct scsi_read_capacity16 rc;
	struct scsi_read_capacity16_data rcd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rc.cmd = SCSI_CMD_READ_CAPACITY16;
	rc.lun = (device->info.lun << SCSI_LUN_SHIFT) | 0x10;
	rc.lba = 0;
	rc.len = sizeof(rcd);
	rc.pmi = 0;
	rc.control = 0;

	r = driver->read(device, (char *)&rc, sizeof(rc),
			 (char *)&rcd, sizeof(rcd));

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	device->info.last_block = betocpu64(rcd.last_block);
	device->info.block_size = betocpu32(rcd.block_size);

	return 0;
}

static int scsi_read10(struct device *device, uint64_t sector, uint32_t size,
		       void *buf)
{
	struct scsi_xfer10 rd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rd.cmd = SCSI_CMD_READ10;
	rd.lun = device->info.lun << SCSI_LUN_SHIFT;
	rd.lba = cputobe32(sector);
	rd._reserved1 = 0;
	rd.size = cputobe16(size);
	rd._reserved2 = 0;
	rd.pad = 0;

	r = driver->read(device, (char *)&rd, sizeof(rd),
			 (char *)buf, size * device->info.block_size);

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	return 0;
}

static int scsi_read12(struct device *device, uint64_t sector, uint32_t size,
           void *buf)
{
	struct scsi_xfer12 rd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rd.cmd = SCSI_CMD_READ12;
	rd.lun = device->info.lun << SCSI_LUN_SHIFT;
	rd.lba = cputobe32(sector);
	rd.size = cputobe32(size);
	rd._reserved = 0;
	rd.control = 0;

	r = driver->read(device, (char *)&rd, sizeof(rd),
			 (char *)buf, size * device->info.block_size);

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	return 0;
}

static int scsi_read16(struct device *device, uint64_t sector, uint32_t size,
           void *buf)
{
	struct scsi_xfer16 rd;
	struct scsi_driver *driver;
	int r;

	driver = scsi_get_driver(device);
	if (!driver)
		return -EFAULT;

	rd.cmd = SCSI_CMD_READ16;
	rd.lun = device->info.lun << SCSI_LUN_SHIFT;
	rd.lba = cputobe64(sector);
	rd.size = cputobe32(size);
	rd._reserved = 0;
	rd.control = 0;

	r = driver->read(device, (char *)&rd, sizeof(rd),
			 (char *)buf, size * device->info.block_size);

	if (scsi_request_sense(device))
		return -EFAULT;

	if (r)
		return r;

	return 0;
}

static int scsi_init(struct device *device)
{
	if (scsi_inquiry(device))
		return -EFAULT;

	if (scsi_read_capacity10(device))
		return -EFAULT;

	if (device->info.last_block == 0xffffffff)
		if (scsi_read_capacity16(device))
			return -EFAULT;

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
		device->device_data = driver;

		return scsi_init(device);
	}

	return -EFAULT;
}

static int scsi_open(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

static int scsi_read(struct device *device, uint64_t sector, uint64_t size, 
		     char *buffer)
{
	int r;

	if (sector >> 32)
		return scsi_read16(device, sector, size, buffer);

	switch (device->info.type) {
		case SCSI_DEVICE_TYPE_DIRECT:
			return scsi_read10(device, sector, size, buffer);
		case SCSI_DEVICE_TYPE_CDROM:
			return  scsi_read12(device, sector, size, buffer);
		default:
			r = -EFAULT;
	}

	return r;
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
	.name = "SCSI",
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