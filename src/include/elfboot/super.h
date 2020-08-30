#ifndef __ELFBOOT_SUPERBLOCK_H__
#define __ELFBOOT_SUPERBLOCK_H__

#include <elfboot/core.h>
#include <elfboot/bdev.h>
#include <elfboot/list.h>

struct fs_node;
struct fs_dent;
struct superblock;

struct superblock_ops {
	struct fs_node *(*alloc_node)(struct superblock *, const char *);
	void (*free_node)(struct fs_node *);
};

struct superblock {
	struct fs_type *fs;

	/*
	 * Backing block device for this superblock. Block devices are exclusive
	 * structures that are able to hold superblocks. The block_size field is
	 * filled by the file systems fill_super function pointer.
	 */
	struct bdev *bdev;
	uint16_t block_size;
	uint16_t block_logs;

	/*
	 * The root member points to the first entry in the superblock while the
	 * mount member points to the entry of the parent node.
	 */
	struct fs_node *root;
	struct fs_node *mount;

	struct superblock_ops *ops;

	/*
	 * Private (file system specific) information. The field can be used for
	 * storing file system superblocks or in general meta data.
	 */
	void *private;
};

#define SUPERBLOCK_SIZE		sizeof(struct superblock)

static inline uint64_t sb_sector(struct superblock *sb, uint64_t sector,
	uint64_t offset)
{
	return sector + (round_down(offset, sb->bdev->block_size) >> sb->bdev->block_logs);
}

static inline uint64_t sb_length(struct superblock *sb, uint64_t offset,
	uint64_t length)
{
	return round_up(offset + length, sb->bdev->block_size) >> sb->bdev->block_logs;
}

struct superblock *superblock_alloc(struct fs_type *fs, struct bdev *bdev);

int superblock_read(struct superblock *sb, uint64_t sector, void *buffer);

int superblock_read_blocks(struct superblock *sb, uint64_t sector,
	uint64_t blknum, void *buffer);

#endif /* __ELFBOOT_SUPERBLOCK_H__ */