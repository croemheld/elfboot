#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/super.h>

int superblock_alloc(struct device *device, struct fs *fs)
{
	device->sb = bmalloc(sizeof(*(device->sb)));
	if (!device->sb)
		return -ENOMEM;

	/* Set references */
	device->sb->device = device;
	device->sb->s_ops  = fs->s_ops;

	return 0;
}

int superblock_probe(struct device *device, struct fs *fs)
{
	if (fs->s_ops->probe)
		return fs->s_ops->probe(device, fs);

	return -ENOTSUP;
}

int superblock_open(struct superblock *sb)
{
	if (sb->s_ops->open)
		return sb->s_ops->open(sb);

	return -ENOTSUP;
}

int superblock_close(struct superblock *sb)
{
	if (sb->s_ops->close)
		return sb->s_ops->close(sb);

	return -ENOTSUP;
}