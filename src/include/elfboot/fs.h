#ifndef __ELFBOOT_FS_H__
#define __ELFBOOT_FS_H__

#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/device.h>
#include <elfboot/super.h>
#include <elfboot/string.h>
#include <elfboot/math.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

#include <uapi/elfboot/common.h>
#include <uapi/elfboot/const.h>

struct fs_dentry {
	char name[128];
	uint64_t offset;
};
#define FS_DENT_SIZE			sizeof(struct fs_dentry)

struct fs_node;

struct fs_ops {
	/* fs_node functions */
	int (*open)(struct fs_node *);
	int (*close)(struct fs_node *);

	/* fs_node functions for directories */
	struct fs_node *(*lookup)(struct fs_node *, const char *);
	struct fs_node *(*mkdir)(struct fs_node *, const char *);
	int (*rmdir)(struct fs_node *, struct fs_dentry *);
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
#define FS_NODE_DIRECTORY_BIT		1
#define FS_NODE_DIRECTORY		_BITUL(FS_NODE_DIRECTORY_BIT)

	uint32_t inode;
	uint64_t offset;
	uint32_t size;
	struct superblock *sb;
	struct fs_ops *ops;

	/* The node in the vfs tree */
	struct tree_node fs_node;
};

#define FS_NODE_SIZE			sizeof(struct fs_node)

struct fs {
	const char *name;
	struct fs_ops *n_ops;
	struct superblock_ops *s_ops;
	struct list_head list;
};

struct fs_lookup_req {
	uint32_t flags;

#define FS_REQUEST_ABSOLUTE	_BITUL(0)

	struct fs_node *node;
	char *path;
};

/*
 * Utility functions
 */

static __always_inline void fs_node_set(struct fs_node *node, uint32_t flags)
{
	node->flags |= flags;
}

static __always_inline bool fs_node_has(struct fs_node *node, uint32_t flags)
{
	return (node->flags & flags) != 0;
}

static __always_inline bool fs_node_is_dir(struct fs_node *node)
{
	return fs_node_has(node, FS_NODE_DIRECTORY);
}

/*
 * Utility functions - node management
 */

static __always_inline void fs_node_add(struct fs_node *child,
					struct fs_node *parent)
{
	if (!child || !parent)
		return;

	tree_node_insert(&child->fs_node, &parent->fs_node);
}

static __always_inline fs_node_parent(struct fs_node *node)
{
	return tree_parent_entry(node, struct fs_node, fs_node);
}

/* 
 * fs_lookup_req initialization
 */

static __always_inline void init_fs_request(struct fs_lookup_req *lookup_req)
{
	memset(lookup_req, 0, sizeof(*lookup_req));
}

/*
 * TODO CRO: Rename and/or place elsewhere
 */

static inline uint64_t calculate_blocks(struct fs_node *node, uint64_t size)
{
	uint64_t blocks;
	uint32_t rest = 0;

	/*
	 * Helper function for determining the number of blocks to 
	 * read and/or write from its underlying device. We need
	 * this because the fs_node cannot pass the actual file size
	 * to the device driver since they use the size parameter
	 * for the number of blocks to read and/or write.
	 */

	blocks = div_u64_rem(size, node->sb->block_size, &rest);
	if (rest)
		blocks++;

	return blocks;
}

struct fs_node *fs_node_alloc(struct fs_node *parent, struct fs_ops *ops,
			      const char *name);

struct fs_node *fs_node_create(struct fs_node *parent, const char *name);

struct fs_node *fs_mkdir(const char *path, uint32_t flags);

int fs_mount(struct device *device, const char *path);

int fs_init(struct device *device);

void fs_register(struct fs *fs);

void fs_unregister(struct fs *fs);

#endif /* __ELFBOOT_FS_H__ */