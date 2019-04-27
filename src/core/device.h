#ifndef __BOOT_DEVICE_H__
#define __BOOT_DEVICE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <list.h>

#include "edd.h"
#include "disk.h"

/*
 * Structure prototype for device
 */

struct device {
	const char *name;
	uint8_t disk_drive;
	struct list_head list;
	struct edd_device_info *edi;
	struct edd_disk_drive_params *params;
	struct disk *disk;
	uint16_t *device_info;
};

static inline int device_is_atapi(struct device *device)
{
	return device->params->drive_options & EDD_DISK_DRIVE_MASK_ATAPI;
}

static inline bool device_is_slave(struct device *device)
{
	return device->params->flags & EDD_DISK_DRIVE_MASK_SLAVE;
}

static inline bool device_is_lba_enabled(struct device *device)
{
	return device->params->flags & EDD_DISK_DRIVE_MASK_LBA;
}

struct device *device_create(uint8_t disk_drive);

#endif /* __BOOT_DEVICE_H__ */