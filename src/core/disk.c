#include <elfboot/disk.h>
#include <elfboot/device.h>

/* List of disk devices */
LIST_HEAD(disk_devices);

/* List of disk firmware */
LIST_HEAD(disk_firmware);

void disk_devices_register(struct disk_operations *ops)
{
	list_add(&ops->list, &disk_devices);
}

void disk_devices_unregister(struct disk_operations *ops)
{
	list_del(&ops->list);
}

void disk_firmware_register(struct disk_operations *ops)
{
	list_add(&ops->list, &disk_firmware);
}

void disk_firmware_unregister(struct disk_operations *ops)
{
	list_del(&ops->list);
}

/* -------------------------------------------------------------------------- */

int disk_probe_firmware2(struct device *device __unused)
{
	struct disk_operations *ops __unused;

	return 0;
}

int disk_probe_firmware(struct device *device)
{
	struct disk_operations *ops;
	const char *interface_type;
	int length;

	interface_type = device->edi->params.interface_type;

	list_for_each_entry(ops, &disk_firmware, list) {

		/* Determine the length of the interface type */
		length = strlen(ops->firmware_name);

		if (!strncmp(ops->firmware_name, interface_type, length)) {
			device->disk->ops = ops;

			return 0;
		}
	}

	return -1;
}

void disk_firmware_init(void)
{
	ata_firmware_init();
}