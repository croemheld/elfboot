#ifndef __ELFBOOT_FS_H__
#define __ELFBOOT_FS_H__

#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/super.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

#define ROOTFS	"ramfs"

#define TREE_NODE_LINE	"\xc3\xc4"
#define TREE_LEAF_LINE	"\xc0\xc4"
#define TREE_PIPE_LINE	"\xb3 "
#define TREE_LINE_SIZE	3

#define for_each_node(pos, root)	\
	tree_for_each_child_entry((pos), (root), tree)
#define get_next_node(node) 		\
	tree_next_sibling_entry_or_null(&(node)->tree, struct fs_node, tree)
#define has_sub_nodes(node)			\
	tree_node_is_leaf(&(node)->tree)

struct tree_info {
	int level;
	struct fs_node *upper;
	struct fs_node *child;
	char *tibuf;
};

struct fs_node;
struct fs_dent;

struct fs_node_ops {
	void (*open)(struct fs_node *);
	void (*close)(struct fs_node *);
	uint32_t (*read)(struct fs_node *, uint64_t, uint32_t, void *);
	uint32_t (*write)(struct fs_node *, uint64_t, uint32_t, const void *);
	struct fs_dent *(*readdir)(struct fs_node *, uint32_t);
	struct fs_node *(*finddir)(struct fs_node *, const char *);
	struct list_head list;
};

struct fs_node {
	char name[64];
	uint32_t mask;
	uint32_t uid;
	uint32_t gid;
	uint32_t flags;
	uint32_t inode;
	uint32_t length;
	uint32_t impl;
	uint32_t open_flags;

	uint32_t atime;
	uint32_t mtime;
	uint32_t ctime;

	struct fs_node_ops *ops;

	struct fs_node *ptr;
	uint32_t refcount;

	union {
		void *device;
		struct bdev *bdev;
		struct cdev *cdev;
	};

	void *private;

	struct superblock *sb;
	struct tree_node tree;
};

#define FS_FILE			_BITUL(0)
#define FS_DIRECTORY	_BITUL(1)
#define FS_CHARDEVICE	_BITUL(2)
#define FS_BLOCKDEVICE	_BITUL(3)
#define FS_PIPE			_BITUL(4)
#define FS_SYMLINK		_BITUL(5)
#define FS_MOUNTPOINT	_BITUL(6)

#define FS_FLAGS_MASK	(_BITUL(7) - 1)

#define FS_NODE_SIZE	sizeof(struct fs_node)
#define FS_NODE_CACHE	"fs_node_cache"

struct fs_dent {
	char name[128];
	uint32_t inode;
};

#define FS_DENT_SIZE	sizeof(struct fs_dent)
#define FS_DENT_CACHE	"fs_dent_cache"

struct fs_type {
	const char *name;
	struct fs_node *(*fill_super)(struct superblock *, const char *);
	struct list_head list;
};

#define FS_LOOKUP_ABSOLUTE	_BITUL(0)

struct fs_lookup_request {
	uint32_t flags;
	char *path;
	struct fs_node *node;
};

static __always_inline struct fs_node *fs_node_parent(struct fs_node *node)
{
	return tree_parent_entry(node, struct fs_node, tree);
}

void vfs_dump_tree(void);

struct fs_node *fs_node_alloc(const char *name);

void fs_register(struct fs_type *fs);

void fs_unregister(struct fs_type *fs);

struct fs_node *vfs_open(const char *path);

void vfs_close(struct fs_node *node);

uint32_t vfs_read(struct fs_node *node, uint64_t off, uint32_t len, void *buf);

uint32_t vfs_write(struct fs_node *node, uint64_t off, uint32_t len,
	const void *buf);

struct fs_dent *vfs_readdir(struct fs_node *node, uint32_t index);

struct fs_node *vfs_finddir(struct fs_node *node, const char *name);

int vfs_mount_type(const char *fs_name, struct bdev *bdev, const char *path,
	const char *name);

int vfs_mount_cdev(struct cdev *cdev, const char *path, const char *name);

int vfs_mount(struct bdev *bdev, const char *path, const char *name);

int vfs_init(void);

#endif /* __ELFBOOT_FS_H__ */