#ifndef __ELFBOOT_SUPERBLOCK_H__
#define __ELFBOOT_SUPERBLOCK_H__

#include <elfboot/core.h>
#include <elfboot/device.h>
#include <elfboot/fs.h>
#include <elfboot/list.h>

struct superblock;

struct superblock_ops {
	int (*probe)(struct device *, struct fs *);
	int (*open)(struct superblock *);
	int (*close)(struct superblock *);
};

struct superblock {

	/*
	 * General information about capacity and
	 * block size of the underlying device.
	 */
	
	uint64_t last_block;
	uint32_t block_size;

	/*
	 * The device which this superblock stores
	 * information about.
	 */
	
	struct device *device;

	/*
	 * The root node of the superblock, i.e.
	 * a mountpoint in the VFS.
	 */

	struct fs_node *root;
	struct superblock_ops *s_ops;
};

int superblock_alloc(struct device *device, struct fs *fs);

int superblock_probe(struct device *device, struct fs *fs);

int superblock_open(struct superblock *sb);

int superblock_close(struct superblock *sb);

#endif /* __ELFBOOT_SUPERBLOCK_H__ */