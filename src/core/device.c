#include <asm/boot.h>

#include "ata.h"
#include "edd.h"
#include "device.h"

/*
 * List of all mounted devices
 */

LIST_HEAD(devices);

int device_open(const char *name, struct device *device)
{
	if (device->disk->ops->open)
		return device->disk->ops->open(name, device);

	return 0;
}

int device_read(struct device *device, uint64_t sector, 
	uint64_t size, char *buffer)
{
	if (device->disk->ops->read)
		return device->disk->ops->read(device, sector, size, buffer);

	return 0;
}

int device_write(struct device *device, uint64_t sector, 
	uint64_t size, const char *buffer)
{
	if (device->disk->ops->write)
		return device->disk->ops->write(device, sector, size, buffer);

	return 0;
}

int device_close(struct device *device)
{
	if (device->disk->ops->close)
		return device->disk->ops->close(device);

	return 0;
}

static int device_init(struct device *device)
{
	void *paramsptr;
	struct edd_device_info *edi;
	struct edd_disk_drive_params *params;

	params = bmalloc(sizeof(*params));

	if (!params)
		return -1;

	edi = bmalloc(sizeof(*edi));

	if (!edi)
		goto device_free_params;

	if (edd_read_device_info(device->disk_drive, edi))
		goto device_free_edi;

	paramsptr = segment_offset_ptr(edi->params.dpte_ptr);
	memcpy(params, paramsptr, sizeof(*params));

	device->edi = edi;
	device->params = params;

	if (disk_probe_firmware(device))
		return -1;

	return 0;

device_free_edi:

	bfree(edi);

device_free_params:

	bfree(params);

	return -1;
}

struct device *device_create(uint8_t disk_drive)
{
	struct device *device;

	device = bmalloc(sizeof(*device));

	if (!device)
		return NULL;

	device->disk_drive = disk_drive;
	
	if (device_init(device)) {

		/*
		 * In device_init all allocations have been freed already.
		 * We only need to free the device structure itself before 
		 * returning.
		 */

		bfree(device);

		return NULL;
	}

	/* Add to list of extisting devices. */
	list_add(&device->list, &devices);

	return device;
}