#ifndef __RAMFS_FS_H__
#define __RAMFS_FS_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#define RAMFS_BLOCK_SIZE	512

void ramfs_fs_init(void);

#endif /* __RAMFS_FS_H__ */