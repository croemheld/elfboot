#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/fs.h>
#include <elfboot/file.h>
#include <elfboot/printf.h>

struct file *file_open(const char *path, uint32_t flags)
{
	struct file *file;

	file = bmalloc(sizeof(*file));
	if (!file)
		return NULL;

	file->node = vfs_open(path);
	if (!file->node)
		goto free_file;

	file->name = bstrdup(file->node->name);
	if (!file->name)
		goto free_file;

	file->flags  = flags;
	file->offset = 0;
	file->length = file->node->length;

	return file;

free_file:
	bfree(file);

	return NULL;
}

void file_close(struct file *file)
{
	if (!file)
		return;

	vfs_close(file->node);
	bfree(file);
}

int file_read(struct file *file, uint32_t nbytes, void *buffer)
{
	uint64_t length = min(nbytes, file->length - file->offset);

	/*
	 * The most important function for reading from files: Make sure that
	 * every file system module implementation allocates a buffer to read
	 * the correct amount of bytes from the file.
	 */

	if (vfs_read(file->node, file->offset, length, buffer))
		return 0;

	return length;
}

int file_write(struct file *file, uint32_t length, const void *buffer)
{
	return 0;
}

int file_lseek(struct file *file, int pos, uint32_t offset)
{
	uint32_t fpos;

	if (offset >= file->length)
		return -1;

	switch (pos) {
		case FILE_SET: fpos = offset;
			break;
		case FILE_CUR:
			if (file->offset + offset >= file->length)
				return -1;

			fpos = file->offset + offset;
			break;
		case FILE_END: fpos = file->length + offset;
			break;
		default:
			fpos = file->offset;
	}

	file->offset = fpos;

	return 0;
}