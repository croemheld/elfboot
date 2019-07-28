#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/linkage.h>
#include <elfboot/string.h>
#include <elfboot/vfs.h>
#include <elfboot/list.h>
#include <elfboot/tree.h>

/*
 * List of available file systems
 */
LIST_HEAD(filesystems);

/*
 * Filesystem root node
 */
static struct fs_node *fs_root;

/*
 * Utility functions
 */

static char *vfs_basename(const char *path)
{
	char *name = strrchr(path, '/');

	/*
	 * This is extremely unsafe, as the path could
	 * contain a trailling '/', which results in
	 * this function returning an empty string.
	 */

	return name ? name + 1 : (char *)path;
}

static char *vfs_dirname(char *path)
{
	/*
	 * vfs_dirname is modifying the path argument.
	 *
	 * If this behavior is not desired, create a copy
	 * of the path before calling this function.
	 */
	
	static const char dot[] = ".";
	char *name;

	name = path != NULL ? strrchr(path, '/') : NULL;

	if (!name)
		return (char *)dot;

	name[0] = '\0';

	return path;
}


/* 
 * fs_node functions
 */

static int vfs_open(struct fs_node *node __unused)
{
	return -ENOTSUP;
}

static int vfs_close(struct fs_node *node __unused)
{
	return -ENOTSUP;
}


/* 
 * fs_node functions for directories
 */

static struct fs_node *vfs_lookup(struct fs_node *node, const char *path)
{
	char *name, *part;

	name = bstrdup(path);
	if (!name)
		return NULL;

	part = strtok(name, "/");

	while (part) {
		if (!node->ops->lookup) {
			bfree(name);
			return NULL;
		}

		node = node->ops->lookup(node, part);
		if (!node) {
			bfree(name);
			return NULL;
		}

		part = strtok(part, "/");
	}

	bfree(name);

	return node;
}

static int vfs_readdir(struct fs_node *node __unused, struct fs_dentry *dentry __unused)
{
	return -ENOTSUP;
}


/* 
 * fs_node function for files
 */

static int vfs_read(struct fs_node *node __unused, uint64_t sector __unused,
		    char *buffer __unused)
{
	return -ENOTSUP;
}

static int vfs_write(struct fs_node *node __unused, uint64_t sector __unused,
		     const char *buffer __unused)
{
	return -ENOTSUP;
}

/* -------------------------------------------------------------------------- */

static int vfs_mount_fs(struct device *device __unused, struct fs *fs __unused,
			const char *name __unused)
{
	return -ENOTSUP;
}

int vfs_mount(struct device *device __unused, const char *name __unused)
{
	return -ENOTSUP;
}

void vfs_register_fs(struct fs *fs)
{
	list_add(&fs->list, &filesystems);
}

void vfs_unregister_fs(struct fs *fs)
{
	list_del(&fs->list);
}