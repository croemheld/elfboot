#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/bdev.h>
#include <elfboot/super.h>
#include <elfboot/string.h>
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

static uint64_t superblock_sectors(struct superblock *sb, uint64_t blkidx)
{
	/*
	 * Calculation of the sector and the number of sectors used to get a single
	 * superblock. This is done by shifting the block logs. Examples:
	 *
	 * sb->block_size = 1024, sb->bdev->block_size = 512
	 *
	 * Superblock 5 equals (5 << 10) >> 9 == Sector 10 on block device.
	 *
	 * The same goes for the number of sectors needed to read one superblock. A
	 * single superblock needs:
	 *
	 * (1 << sb->block_logs) >> sb->bdev->block_logs sectors. Example:
	 *
	 * (1 << 10) >> 9 == 2 sectors to read a single superblock of 1024 bytes.
	 */
	return (blkidx << sb->block_logs) >> sb_bdlogs(sb);
}

/*
 * TODO CRO: Make it inline!
 */

static uint64_t superblock_blockid(struct superblock *sb, uint64_t offset)
{
	return (offset >> sb->block_logs);
}

static uint64_t superblock_blockno(struct superblock *sb, uint64_t blkmod,
	uint64_t length)
{
	uint64_t blklog = sb->block_logs;

	return ((blkmod + length - 1) >> blklog) - (blkmod >> blklog) + 1;
}

int superblock_read_block(struct superblock *sb, uint64_t blkidx, void *buffer)
{
	uint64_t sector, secnum;

	sector = superblock_sectors(sb, blkidx);
	secnum = superblock_sectors(sb, 1);

	return bdev_read(sb->bdev, sector, secnum, buffer);
}

int superblock_read_blocks(struct superblock *sb, uint64_t blkidx,
	uint64_t blknum, void *buffer)
{
	uint64_t blk;

	for (blk = 0; blk < blknum; blk++) {
		/*
		 * Read a continuous number of superblocks form the underlying device
		 * into a buffer. The buffer is allocated by the caller. The function
		 * only works if all superblocks are contiguous.
		 */
		if (superblock_read_block(sb, blkidx + blk, buffer + blk * sb->block_size))
			return -EFAULT;
	}

	return 0;
}

int superblock_read_offset(struct superblock *sb, uint64_t offset,
	uint64_t length, void *buffer)
{
	uint64_t blkidx, blknum, blkoff, *blkbuf;
	uint32_t blkmod, remlen, bufoff = 0;

	blkidx = div(offset, sb->block_size, &blkmod);
	blknum = superblock_blockno(sb, blkmod, length);
	blkbuf = bmalloc(sb->block_size);
	if (!blkbuf)
		return -ENOMEM;

	for (blkoff = 0; blkoff < blknum; blkoff++) {
		if (superblock_read_block(sb, blkidx + blkoff, blkbuf))
			goto superblock_free_blkbuf;

		if (blkoff)
			blkmod = 0;

		remlen  = min(sb->block_size - blkmod, length);
		memcpy(buffer + bufoff, vptradd(blkbuf, blkmod), remlen);

		bufoff += remlen;
		length -= remlen;
	}

	bfree(blkbuf);

	return 0;

superblock_free_blkbuf:
	bfree(blkbuf);

	return -EFAULT;
}
