#ifndef __ELFBOOT_MEMBLOCK_H__
#define __ELFBOOT_MEMBLOCK_H__

#include <elfboot/mm.h>

#define MEMBLOCK_MAX_REGIONS		32

#define MEMBLOCK_START			MEMORY_START
#define MEMBLOCK_LIMIT			MEMORY_LIMIT

#define MEMBLOCK_INVLD			(int)0xffffffff

struct memblock_region {
	uint32_t base;
	uint32_t size;
};

#define memblock_beg(region)		((region)->base)
#define memblock_end(region)		((region)->base + (region)->size)

struct memblock_type {
	uint32_t cnt;
	uint32_t max;
	uint32_t total_size;
	struct memblock_region *regions;
};

struct memblock {
	struct memblock_type memory;
};

extern struct memblock memblock;

#define for_each_memblock_type(i, type, region)			\
	for (i = 0, region = &type->regions[0];			\
	     i < type->cnt;					\
	     i++, region = &type->regions[i])

#define for_each_free_memblock(i, region)			\
	for_each_memblock_type(i, (&memblock.memory), region)

void memblock_add(uint32_t base, uint32_t size);

int memblock_reserve(uint32_t base, uint32_t size);

void *memblock_alloc(uint32_t size, uint32_t align);

#endif /* __ELFBOOT_MEMBLOCK_H__ */