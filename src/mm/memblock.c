#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

static struct memblock_region bootmem[MEMBLOCK_MAX_REGIONS];
static struct memblock_region kernmem[MEMBLOCK_MAX_REGIONS];

struct memblock memblock = {
	.bootmem.cnt = 1,
	.bootmem.max = MEMBLOCK_MAX_REGIONS,
	.bootmem.regions = bootmem,
	.kernmem.cnt = 1,
	.kernmem.max = MEMBLOCK_MAX_REGIONS,
	.kernmem.regions = kernmem
};

#ifdef CONFIG_DEBUG_MM

static void __memblock_dump(struct memblock_type *type)
{
	uint32_t i, base, size, rend;
	struct memblock_region *region;

	bprintln("Number of memblock regions: %u", type->cnt);

	for_each_memblock_type(i, type, region) {
		base = region->base;
		size = region->size;
		rend = base + size - 1;

		bprintln("[%08lx - %08lx], %08lx bytes", base, rend, size);
	}
}

void memblock_dump(void)
{
	__memblock_dump(&memblock.bootmem);
}

#endif /* CONFIG_DEBUG_MM */

static uint32_t memblock_adjust_region(uint32_t *base, uint32_t *size)
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

	return *base + *size;
}

static void memblock_fusion_region(struct memblock_type *type, 
	struct memblock_region *region, uint32_t index)
{
	struct memblock_region *prvreg, *nxtreg;

	if (type->cnt == 1)
		return;

	if (index) {
		prvreg = region - 1;

		if (memblock_end(prvreg) == memblock_beg(region)) {
			region->base  = prvreg->base;
			region->size += prvreg->size;

			memmove(prvreg, region, (type->cnt - index) * sizeof(*region));
		}
	}

	/*
	 * A single memblock can be merged with both of its neighbours which is
	 * why we also have to check whether the successor memblock is adjacent
	 * to the current memblock region.
	 */

	if (index < type->cnt) {
		nxtreg = region + 1;

		if (memblock_end(region) == memblock_beg(nxtreg)) {
			nxtreg->base  = region->base;
			nxtreg->size += region->size;

			memmove(region, nxtreg, (type->cnt - (index + 1)) * sizeof(*region));
		}
	}
}

static void memblock_insert_region(struct memblock_type *type, uint32_t index,
	uint32_t base, uint32_t size)
{
	struct memblock_region *region = &type->regions[index];

	/*
	 * Move all existing memblock regions one place to the right. That way,
	 * the region pointer points to our new memblock region which we update
	 * with the given base and size.
	 */
	memmove(region + 1, region, (type->cnt - index) * sizeof(*region));

	region->base = base;
	region->size = size;
	type->total_size += size;
	type->cnt++;
}

static void memblock_add_region(struct memblock_type *type, uint32_t base,
	uint32_t size)
{
	uint32_t i, rbeg, rend, nend;
	struct memblock_region *region;

	nend = memblock_adjust_region(&base, &size);

	region = &type->regions[0];
	if (region->size == 0) {
		region->base = base;
		region->size = size;
		type->total_size += size;
		return;
	}

	for_each_memblock_type(i, type, region) {
		rbeg = memblock_beg(region);
		rend = memblock_end(region);

		if (rbeg >= nend)
			break;
		if (rend <= base)
			continue;

		if (rbeg >= base)
			memblock_insert_region(type, i++, base, rbeg - base);

		base = min(rend, nend);
	}

	if (base < nend)
		memblock_insert_region(type, i, base, nend - base);

	/*
	 * Check if the new block can be fused with its neighbours.
	 */
	memblock_fusion_region(type, &type->regions[i], i);
}

static int memblock_del_region(struct memblock_type *type, uint32_t base,
	uint32_t size)
{
	uint32_t i, rbeg, rend, nend, mbeg, mend, dbeg;
	struct memblock_region *region;

	dbeg = MEMBLOCK_INVLD;
	nend = memblock_adjust_region(&base, &size);

	for_each_memblock_type(i, type, region) {
		rbeg = memblock_beg(region);
		rend = memblock_end(region);

		if (rbeg >= nend)
			return dbeg;
		if (rend <= base)
			continue;

		mbeg = max(rbeg, base);
		mend = min(rend, nend);
		if (!dbeg)
			dbeg = mbeg;

		/*
		 * In a special case: The memory region to be removed is will cause the
		 * current memblock region to be split into two memblock regions
		 */
		if (rbeg < base && rend > nend) {
			region->size = base - rbeg;
			memblock_insert_region(type, i + 1, nend, rend - nend);
			return base;
		}

		region->size -= mend - mbeg;
		type->total_size -= mend - mbeg;

		if (!region->size) {
			memmove(region, region + 1, (type->cnt - (i-- + 1)) * sizeof(*region));
			type->cnt--;
			continue;
		}

		if (rbeg >= base) {
			region->base = nend;
			return base;
		}

		if (rend <= nend) {
			base = rend;
			size = nend - rend;
			continue;
		}
	}

	return dbeg;
}

void memblock_add(uint32_t base, uint32_t size)
{
	memblock_add_region(&memblock.bootmem, base, size);
}

void memblock_add_kernel(uint32_t base, uint32_t size)
{
	memblock_add_region(&memblock.kernmem, base, size);
}

static int __memblock_reserve(struct memblock_type *type,
	uint32_t base, uint32_t size)
{
	return memblock_del_region(type, base, size) != MEMBLOCK_INVLD;
}

int memblock_reserve(uint32_t base, uint32_t size)
{
	return memblock_del_region(&memblock.bootmem, base, size) != MEMBLOCK_INVLD;
}

static uint32_t memblock_find_in_range(struct memblock_type *type, 
	uint32_t size, uint32_t align, uint32_t start, uint32_t end)
{
	uint32_t rbeg, rend, cand;
	struct memblock_region *region;
	uint32_t i;

	for_each_memblock_type(i, type, region) {
		rbeg = clamp(memblock_beg(region), start, end);
		rend = clamp(memblock_end(region), start, end);

		cand = round_up(rbeg, align);
		if (cand < rend && rend - cand >= size)
			return cand;
	}

	return 0;
}

static uint32_t memblock_alloc_range(struct memblock_type *type, 
	uint32_t size, uint32_t align, uint32_t start, uint32_t end)
{
	uint32_t base = memblock_find_in_range(type, size, align, start, end);

	if (base && __memblock_reserve(type, base, size))
		return base;

	return 0;
}

static void *memblock_alloc_internal(struct memblock_type *type, 
	uint32_t size, uint32_t align, uint32_t start, uint32_t end)
{
	uint32_t base = memblock_alloc_range(type, size, align, start, end);

	if (!base)
		return NULL;

	return tvptr(base);
}

void *memblock_alloc(uint32_t size, uint32_t align)
{
	/*
	 * We only support allocations below the 1MB mark since we
	 * want to load kernels starting at addresses > 1MB.
	 */
	
	return memblock_alloc_internal(&memblock.bootmem, size, align,
		MEMBLOCK_START, MEMBLOCK_LIMIT);
}

void *memblock_alloc_kernel(uint32_t size, uint32_t align)
{
	/*
	 * This function is intented to be used for the allocation
	 * of kernel and kernel module pages only.
	 */

	return memblock_alloc_internal(&memblock.kernmem, size, align,
		MEMBLOCK_LIMIT, MEMBLOCK_INVLD);
}