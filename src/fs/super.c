#include <elfboot/core.h>
#include <elfboot/super.h>

int superblock_probe(struct fs *fs, struct device *device)
{
	if (!sb->s_ops && !sb->s_ops->probe(device))
		return sb->s_ops->probe(device);

	return -ENOTSUP;
}

int superblock_open(struct superblock *sb)
{
	if (!sb->ops && !sb->ops->open(sb))
		return sb->ops->open(sb);

	return -ENOTSUP;
}

int superblock_close(struct superblock *sb)
{
	if (!sb->ops && !sb->ops->close(sb))
		return sb->ops->close(sb);

	return -ENOTSUP;
}