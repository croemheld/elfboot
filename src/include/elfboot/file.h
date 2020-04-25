#ifndef __ELFBOOT_FILE_H__
#define __ELFBOOT_FILE_H__

#include <elfboot/core.h>
#include <elfboot/fs.h>

struct file {
	char *name;
	struct fs_node *node;
	uint32_t flags;
	uint32_t offset;
	uint32_t length;
};

enum file_lseek {
	FILE_SET,
	FILE_CUR,
	FILE_END
};

struct file *file_open(const char *path, uint32_t flags);

void file_close(struct file *file);

int file_read(struct file *file, uint32_t length, void *buffer);

int file_write(struct file *file, uint32_t length, const void *buffer);

int file_lseek(struct file *file, int pos, uint32_t offset);

#endif /* __ELFBOOT_FILE_H__ */