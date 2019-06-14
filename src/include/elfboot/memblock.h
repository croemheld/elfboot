#ifndef __ELFBOOT_MEMBLOCK_H__
#define __ELFBOOT_MEMBLOCK_H__

struct memblock_region {
	uint32_t base;
	uint32_t size;
};

struct memblock_type {
	uint32_t cnt;
	uint32_t max;
	uint32_t total_size;
	struct memblock_region *regions;
};

struct memblock {
	struct memblock_type memory;
};

int memblock_add(uint32_t base, uint32_t size);

int memblock_reserve(uint32_t base, uint32_t size);

#endif /* __ELFBOOT_MEMBLOCK_H__ */