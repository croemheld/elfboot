#ifndef __BOOT_ALLOC_H__
#define __BOOT_ALLOC_H__

#include <elfboot/core.h>
#include <elfboot/printf.h>
#include <elfboot/string.h>
#include <elfboot/list.h>

#include <uapi/elfboot/common.h>

struct alloc_node {
	struct list_head node;
	size_t size;
	void *data;
};

#define ALLOC_NODE_HDR_SIZE                    offsetof(struct alloc_node, data)

#endif /* __BOOT_ALLOC_H__ */