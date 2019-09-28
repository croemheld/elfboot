#ifndef __ELFBOOT_SLUB_H__
#define __ELFBOOT_SLUB_H__

#include <elfboot/core.h>

#include <uapi/elfboot/const.h>

struct bmem_info {
	const char *name;
	uint32_t size;
};

#define KMALLOC_CACHE_NUM	13

#define KMALLOC_MIN_SHIFT	 2
#define KMALLOC_MIN_SIZE	32

/*
 * We make several shortcuts compared to the Linux implementation
 * for the SLUB allocator.
 * 
 * One: We only support the creation of
 * slabs with a size of a power of 2. That way, we won't need to
 * use any alignments, slab colorings, etc.
 *
 * Second: We don't use GFP flags for allocations as we won't need
 * them in a bootloader.
 *
 * Third: We combine the kmem_cache and kmem_cache_node structures
 * in one single structure because we don't have a NUMA model and
 * no multiple CPUs.
 *
 * Fourth: The freelist pointer used in the Linux kernel is used
 * differently: We use a list_head structure instead and link free
 * objects together. That way we don't need so allocate freelists
 * in order to keep track of free objects for allocations.
 */

struct bmem_cache {
	const char *name;
	uint32_t size;
	uint32_t nums;
	uint32_t order;
	uint32_t free_objs;
	uint32_t free_slabs;
	uint32_t total_slabs;
	struct list_head slabs_full;
	struct list_head slabs_partial;
	struct list_head slabs_free;
	void (*ctor)(void *);
	struct list_head next;
};

void slab_dump(void);

void *bmem_cache_alloc(struct bmem_cache *cachep);

struct bmem_cache *bmem_cache_create(const char *name, uint32_t size,
				     void (*ctor)(void *));

#endif /* __ELFBOOT_SLUB_H__ */