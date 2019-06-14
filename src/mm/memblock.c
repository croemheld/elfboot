#include <elfboot/core.h>
#include <elfboot/mm.h>

static inline uint32_t memblock_cap_size(uint32_t base, uint32_t *size)
{
	return *size = min(*size, PHYS_ADDR_MAX - base);
}

static void memblock_merge_regions(struct memblock_type *type)
{
	int i = 0;
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

static int memblock_add_range(struct memblock_type *type, uint32_t base,
			      uint32_t size)
{
	int nr_new;
	struct memblock_region *region;
	uint32_t idx, rbase, rend, end = base + memblock_cap_size(base, &size);

	if (!size)
		return 0;

	if (type->regions[0].size == 0) {
		type->regions[0].base = base;
		type->regions[0].size = size;
		type->total_size = size;
		return 0;
	}

	nr_new = 0;

	for_each_memblock_type(idx, type, region) {
		rbase = region->base;
		rend = rbase + region->size;

		if (rbase >= end)
			break;
		if (rend <= base)
			continue;
		/*
		 * @region overlaps.  If it separates the lower part of new
		 * area, insert that portion.
		 */
		if (rbase > base) {
			nr_new++;
			memblock_insert_region(type, idx++, base, rbase - base);
		}
		/* area below @rend is dealt with, forget about it */
		base = min(rend, end);
	}

	/* insert the remaining portion */
	if (base < end) {
		nr_new++;
		memblock_insert_region(type, idx, base, end - base);
	}

	if (!nr_new)
		return 0;

	memblock_merge_regions(type);
}

static int memblock_isolate_range(struct memblock_type *type,
				  uint32_t base, uint32_t size,
				  int *start_rgn, int *end_rgn)
{
	uint32_t idx;
	struct memblock_region *region;
	uint32_t end = base + memblock_cap_size(base, &size);

	*start_rgn = *end_rgn = 0;

	if (!size)
		return 0;

	for_each_memblock_type(idx, type, region) {
		uint32_t rbase = region->base;
		uint32_t rend = rbase + region->size;

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