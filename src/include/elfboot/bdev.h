#ifndef __ELFBOOT_BDEV_H__
#define __ELFBOOT_BDEV_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

#define BDEV_FLAGS_BOOT	_BITUL(0)
#define BDEV_FLAGS_LBA	_BITUL(1)

/*
 * Block devices
 */

struct bdev;

struct bdev_ops {
	int (*read)(struct bdev *, uint64_t, uint64_t, void *);
	int (*write)(struct bdev *, uint64_t, uint64_t, const void *);
	int (*ioctl)(struct bdev *, int, void *);

	/* List of bdev drivers */
	struct list_head list;
};

struct bdev {
	const char *name;

	/*
	 * Block devices can be anything that stores data in blocks. For ramdisks
	 * the blocks represent contiguous regions in physical memory, for actual
	 * devices like ATA or SCSI, the blocks are addressed via CHS or LBA.
	 *
	 * The flags indicate what kind of device this structure represents. When
	 * the device supports both CHS and LBA addressing mode, we automatically
	 * use LBA since it is easier.
	 */
	uint32_t flags;

	/*
	 * Device-specific private information. This is used by devices which are
	 * nested, i.e.: block device -> PCI IDE controller.
	 */
	void *private;

	/*
	 * Some block device use a LUN as an identifier
	 */
	uint64_t lun;

	/*
	 * Addressing mode values. Save space by unifying nameless structures. If
	 * the device can use either addressing mode, always use LBA values.
	 */
	union {
		/* CHS */
		struct {
			uint32_t cylinders;
			uint32_t heads;
			uint32_t sectors_per_track;
			uint32_t total_sectors;
		};

		/* LBA */
		struct {
			uint64_t init_block;
			uint64_t last_block;
		};
	};

	uint16_t block_size;
	uint16_t block_logs;

	/*
	 * Operations for this device. The functions can only be retrieved from a
	 * module which implements block device functions.
	 */
	struct bdev_ops *ops;

	/*
	 * List of initialized devices in the list_head structure.
	 */
	struct list_head list;
};

/*
 * Block device functions
 */

static inline uint64_t bdev_blknum(struct bdev *bdev, uint64_t length)
{
	return ((length - 1) >> bdev->block_logs) + 1;
}

int bdev_read(struct bdev *bdev, uint64_t sector,
	uint64_t blknum, void *buffer);

int bdev_write(struct bdev *bdev, uint64_t sector,
	uint64_t blknum, const void *buffer);

int bdev_ioctl(struct bdev *bdev, int request, void *args);

void bdev_register_driver(struct bdev_ops *ops);

struct bdev *bdev_get(uint32_t flags, struct bdev *from);

int bdev_init(struct bdev *bdev, struct bdev_ops *ops);

#endif /* __ELFBOOT_BDEV_H__ */