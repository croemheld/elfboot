#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/bdev.h>
#include <elfboot/super.h>
#include <elfboot/printf.h>

struct superblock *superblock_alloc(struct fs_type *fs, struct bdev *bdev)
{
	struct superblock *sb;

	sb = bmalloc(SUPERBLOCK_SIZE);
	if (!sb)
		return NULL;

	sb->block_logs = bdev->block_logs;
	sb->block_size = bdev->block_size;

	sb->fs = fs;
	sb->bdev = bdev;

	return sb;
}

int superblock_read(struct superblock *sb, uint64_t sector, void *buffer)
{
	return bdev_read(sb->bdev, sector, 1, buffer);
}

int superblock_read_blocks(struct superblock *sb, uint64_t sector,
	uint64_t blknum, void *buffer)
{
	uint64_t blk;

	for (blk = 0; blk < blknum; blk++) {

		/*
		 * The buffer is expected to be (sb->block_size * numblk) in size and
		 * trimming of the buffer is done by the caller. This function simply 
		 * calls superblock_read multiple times.
		 */
		if (superblock_read(sb, sector + blk, buffer + (blk * sb->block_size)))
			return -EFAULT;
	}

	return 0;
}