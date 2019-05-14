#ifndef __BOOT_ALLOC_H__
#define __BOOT_ALLOC_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <asm/printf.h>
#include <asm/memory.h>

#include <elfboot/string.h>

#include <uapi/elfboot/common.h>

#include <list.h>

struct alloc_node {
	struct list_head node;
	size_t size;
	void *data;
};

#define ALLOC_NODE_HDR_SIZE                    offsetof(struct alloc_node, data)
#define ALLOC_NODE_MIN_SIZE                       ALLOC_NODE_HDR_SIZE + 32

#endif /* __BOOT_ALLOC_H__ */