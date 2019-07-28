#ifndef __ELFBOOT_VFS_H__
#define __ELFBOOT_VFS_H__

#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/string.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

#include <uapi/elfboot/common.h>
#include <uapi/elfboot/const.h>

struct fs_dentry {
	uint8_t foo;
};

struct fs_node;

struct fs_ops {
	/* fs_node functions */
	int (*open)(struct fs_node *);
	int (*close)(struct fs_node *);

	/* fs_node functions for directories */
	struct fs_node *(*lookup)(struct fs_node *, const char *);
	int (*readdir)(struct fs_node *, struct fs_dentry *);

	/* fs_node function for files */
	int (*read)(struct fs_node *, uint64_t, char *);
	int (*write)(struct fs_node *, uint64_t, const char *);
};

struct fs_node {
	char name[128];
	uint32_t flags;

#define FS_NODE_MOUNT_BIT		0
#define FS_NODE_MOUNT			_BITUL(FS_NODE_MOUNT_BIT)
#define FS_NODE_FILE_BIT		1
#define FS_NODE_FILE			_BITUL(FS_NODE_FILE_BIT)
#define FS_NODE_DIRECTORY_BIT		2
#define FS_NODE_DIRECTORY		_BITUL(FS_NODE_DIRECTORY_BIT)

	uint32_t inode;
	struct superblock *sb;
	struct fs_ops *ops;

	/* The node in the vfs tree */
	struct tree_node fs_node;
};

struct fs {
	const char *name;
	struct fs_ops *n_ops;
	struct superblock_ops *s_ops;
	struct list_head list;
};

static inline void fs_node_set_flags(struct fs_node *node, uint32_t flags)
{
	node->flags |= flags;
}

void vfs_register_fs(struct fs *fs);

void vfs_unregister_fs(struct fs *fs);

#endif /* __ELFBOOT_VFS_H__ */