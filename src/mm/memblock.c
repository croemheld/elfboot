#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

static struct memblock_region memory[MEMBLOCK_MAX_REGIONS];

struct memblock memblock = {
	.memory.cnt = 1,
	.memory.max = MEMBLOCK_MAX_REGIONS,
	.memory.regions = memory
};

static void __memblock_dump(struct memblock_type *type)
{
	uint32_t i, base, size, rend;
	struct memblock_region *region;

	bprintln("Number of memblock regions: %u", type->cnt);

	for_each_memblock_type(i, type, region) {
		base = region->base;
		size = region->size;
		rend = base + size - 1;

		bprintln("[%08p - %08p], %08p bytes", base, rend, size);
	}
}

void memblock_dump(void)
{
	__memblock_dump(&memblock.memory);
}

static inline uint32_t memblock_cap_size(uint32_t base, uint32_t *size)
{
	/* 
	 * If the size goes over the maximum of the physical address space, 
	 * then use the remaining size as the new size for our region.
	 */
	return *size = min(*size, (uint32_t)(~0ULL) - base);
}

static void memblock_adjust_range(uint32_t *base, uint32_t *size)
{
	uint32_t nbase, limit;

	/*
	 * The memblock allocator only supports the management of page-sized
	 * memory chunks. Therefore, all base and size values passed to the
	 * public functions need to be trimmed down or rounded up.
	 */

	nbase = round_down(*base, PAGE_SIZE);
	limit = round_up(*base + *size, PAGE_SIZE);

	*base = nbase;
	*size = limit - nbase;
}

static void memblock_merge_regions(struct memblock_type *type)
{
	uint32_t i = 0;
	struct memblock_region *this, *next;

	while (i < type->cnt - 1) {
		this = &type->regions[i];
		next = &type->regions[i + 1];

		if (this->base + this->size != next->base) {
			i++;
			continue;
		}

		this->size += next->size;

		memmove(next, next + 1, (type->cnt - (i + 2)) * sizeof(*next));
		type->cnt--;
	}
}

static void memblock_insert_region(struct memblock_type *type, int idx, 
				   uint32_t base, uint32_t size)
{
	struct memblock_region *region = &type->regions[idx];

	memmove(region + 1, region, (type->cnt - idx) * sizeof(*region));
	region->base = base;
	region->size = size;
	type->cnt++;
	type->total_size += size;
}

static void memblock_remove_region(struct memblock_type *type, uint32_t r)
{
	type->total_size -= type->regions[r].size;
	memmove(&type->regions[r], &type->regions[r + 1],
		(type->cnt - (r + 1)) * sizeof(type->regions[r]));
	type->cnt--;

	/* Special case for empty arrays */
	if (type->cnt == 0) {
		type->cnt = 1;
		type->regions[0].base = 0;
		type->regions[0].size = 0;
	}
}

static int memblock_add_range(struct memblock_type *type, uint32_t base,
			      uint32_t size)
{
	int count;
	struct memblock_region *region;
	uint32_t idx, rbase, rend, end;

	/* Adjust to page granularity */
	memblock_adjust_range(&base, &size);
	end = base + memblock_cap_size(base, &size);

	if (!size)
		return 0;

	if (type->regions[0].size == 0) {
		type->regions[0].base = base;
		type->regions[0].size = size;
		type->total_size = size;
		return 0;
	}

	count = 0;

	for_each_memblock_type(idx, type, region) {
		rbase = region->base;
		rend = rbase + region->size;

		if (rbase >= end)
			break;

		if (rend <= base)
			continue;

		if (rbase > base) {
			count++;
			memblock_insert_region(type, idx++, base, rbase - base);
		}

		base = min(rend, end);
	}

	if (base < end) {
		count++;
		memblock_insert_region(type, idx, base, end - base);
	}

	if (!count)
		return 0;

	memblock_merge_regions(type);

	return count;
}

static int memblock_isolate_range(struct memblock_type *type,
				  uint32_t base, uint32_t size,
				  int *start_rgn, int *end_rgn)
{
	uint32_t idx, end, rbase, rend;
	struct memblock_region *region;

	*start_rgn = *end_rgn = 0;
	end = base + memblock_cap_size(base, &size);

	if (!size)
		return 0;

	for_each_memblock_type(idx, type, region) {
		rbase = region->base;
		rend  = rbase + region->size;

		if (rbase >= end)
			break;
		if (rend <= base)
			continue;

		if (rbase < base) {
			region->base = base;
			region->size -= base - rbase;
			type->total_size -= base - rbase;
			memblock_insert_region(type, idx, rbase, base - rbase);
		} else if (rend > end) {
			region->base = end;
			region->size -= end - rbase;
			type->total_size -= end - rbase;
			memblock_insert_region(type, idx--, rbase, end - rbase);
		} else {
			if (!*end_rgn)
				*start_rgn = idx;
			*end_rgn = idx + 1;
		}
	}

	return 0;
}

static int memblock_remove_range(struct memblock_type *type,
				 uint32_t base, uint32_t size)
{
	int i, ret, start_rgn, end_rgn;

	/* Adjust to page granularity */
	memblock_adjust_range(&base, &size);

	ret = memblock_isolate_range(type, base, size, &start_rgn, &end_rgn);
	if (ret)
		return ret;

	for (i = end_rgn - 1; i >= start_rgn; i--)
		memblock_remove_region(type, i);

	return 0;
}

int memblock_add(uint32_t base, uint32_t size)
{
	return memblock_add_range(&memblock.memory, base, size);
}

int memblock_reserve(uint32_t base, uint32_t size)
{
	return memblock_remove_range(&memblock.memory, base, size);
}

static uint32_t memblock_find_in_range(uint32_t size, uint32_t align,
				       uint32_t start, uint32_t end)
{
	uint32_t rbase, rend, cand;
	struct memblock_region *region;
	uint32_t i;

	for_each_free_memblock(i, region) {
		rbase = clamp(region->base, start, end);
		rend  = clamp(memblock_end(region), start, end);

		cand = round_up(rbase, align);
		if (cand < rend && rend - cand >= size)
			return cand;
	}

	return 0;
}

static uint32_t memblock_alloc_range(uint32_t size, uint32_t align,
				     uint32_t start, uint32_t end)
{
	uint32_t base = memblock_find_in_range(size, align, start, end);

	if (base && !memblock_reserve(base, size))
		return base;

	return 0;
}

static void *memblock_alloc_internal(uint32_t size, uint32_t align,
				     uint32_t start, uint32_t end)
{
	uint32_t base = memblock_alloc_range(size, align, start, end);

	if (!base)
		return NULL;

	return uinttvptr(base);
}

void *memblock_alloc(uint32_t size, uint32_t align)
{
	/*
	 * We only support allocations below the 1MB mark since we
	 * want to load kernels starting at addresses > 1MB.
	 */
	
	return memblock_alloc_internal(size, align, BOOT_END, MEMBLOCK_LIMIT);
}