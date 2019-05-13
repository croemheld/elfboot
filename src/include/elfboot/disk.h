#ifndef __BOOT_DISK_FIRMWARE_H__
#define __BOOT_DISK_FIRMWARE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <elfboot/ata.h>

#include <list.h>

#define DISK_SECTOR_SIZE                          0x200

struct device;

struct disk_operations {
	const char *firmware_name;
	struct list_head list;
	int (*open)(const char *, struct device *);
	int (*read)(struct device *, uint64_t, uint64_t, char *);
	int (*write)(struct device *, uint64_t, uint64_t, const char *);
	int (*close)(struct device *);
};

struct disk {
	const char *name;
	struct disk_operations *ops;
	uint64_t total_sectors;
	uint32_t log_sector_size;
	uint32_t max_agglomerate;
	void *data;
};

void disk_devices_register(struct disk_operations *ops);

void disk_devices_unregister(struct disk_operations *ops);

void disk_firmware_register(struct disk_operations *ops);

void disk_firmware_unregister(struct disk_operations *ops);

int disk_probe_firmware(struct device *device);

void disk_firmware_init(void);

#endif /* __BOOT_DISK_FIRMWARE_H__ */