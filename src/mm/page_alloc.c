#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/printf.h>
#include <elfboot/string.h>
#include <elfboot/math.h>
#include <elfboot/list.h>

#define PAGE_MAX_ORDER		9
#define PAGE_MAX_ALLOC_SIZE	((1UL << (PAGE_MAX_ORDER - 1)) * PAGE_SIZE)

/* -------------------------------------------------------------------------- */

/* List of free pages */
static struct free_area free_area[PAGE_MAX_ORDER];

/* Number of pages */
static uint32_t page_num = 0;

/* Structure page map */
struct page *page_map;

/*
 * Utility functions
 */

#ifdef CONFIG_DEBUG_MM

void page_dump(void)
{
	bprintln("Order %3lu %3lu %3lu %3lu %3lu %3lu %3lu %3lu %3lu", 
		0, 1, 2, 3, 4, 5, 6, 7, 8);
	bprintln("Frees %3lu %3lu %3lu %3lu %3lu %3lu %3lu %3lu %3lu",
		free_area[0].nr_free, free_area[1].nr_free, free_area[2].nr_free, 
		free_area[3].nr_free, free_area[4].nr_free, free_area[5].nr_free, 
		free_area[6].nr_free, free_area[7].nr_free, free_area[8].nr_free);
	bprintln("-----------------------------------------");
}

#endif /* CONFIG_DEBUG_MM */

static inline bool page_is_aligned(uint32_t addr, uint32_t order)
{
	return (addr % (PAGE_SIZE << order) == 0);
}

static inline struct free_area *page_free_area_order(uint32_t order)
{
	return &free_area[order];
}

static inline struct free_area *page_free_area(struct page *page)
{
	return page_free_area_order(page->order);
}

static void free_page_add(struct page *page, uint32_t order)
{
	struct free_area *free_area = page_free_area_order(order);

	list_add(&page->list, &free_area->list);
	free_area->nr_free++;
}

static void free_page_append(struct page *page, uint32_t order)
{
	struct free_area *free_area = page_free_area_order(order);

	list_add_tail(&page->list, &free_area->list);
	free_area->nr_free++;
}

static void free_page_del(struct page *page)
{
	struct free_area *free_area = page_free_area(page);

	list_del(&page->list);
	free_area->nr_free--;
}

static struct page *free_area_get_first(uint32_t order)
{
	struct free_area *free_area = page_free_area_order(order);

	return list_first_entry_or_null(&free_area->list, struct page, list);
}

/*
 * Allocating pages
 */

static void page_set_compound_head(struct page *page, uint32_t order)
{
	uint32_t pfn, nr_pages = (1 << order);

	/*
	 * By setting the order at the compound head page we
	 * can determine on which order all pages of the same
	 * set were allocated. This is easily done by getting
	 * the order of the compound head page.
	 */
	
	for (pfn = 0; pfn < nr_pages; pfn++)
		(page + pfn)->compound_head = page;
}

static void free_page_fix(struct page *page, uint32_t order)
{
	struct page *buddy;
	uint32_t porder;

	porder = page->order;

	while (porder > order) {
		buddy = page_buddy(page, --porder);
		buddy->order = porder;
		buddy->flags |= PAGE_FLAG_FREE;

		free_page_add(buddy, porder);
	}
}

static struct page *get_free_pages(uint32_t order)
{
	struct page *buddy, *page = NULL;
	uint32_t porder;

	for (porder = order; porder < PAGE_MAX_ORDER; porder++) {
		page = free_area_get_first(porder);

		if (!page)
			continue;

		free_page_del(page);
		free_page_fix(page, order);

		page->flags &= ~PAGE_FLAG_FREE;
		page->order = order;

		break;
	}

	if (!page)
		return NULL;

	/* Set the head page in all pages */
	page_set_compound_head(page, order);

	buddy = page_buddy(page, order);
	page_set_compound_head(buddy, order);

	return page;
}

struct page *alloc_pages(uint32_t order)
{
	return get_free_pages(order);
}

struct page *alloc_page(void)
{
	return alloc_pages(0);
}

void *get_zeroed_page(void)
{
	struct page *page = alloc_page();

	if (!page)
		return NULL;

	memset(page_address(page), 0, PAGE_SIZE);

	return page_address(page);
}

/*
 * Freeing pages
 */

static struct page *merge_pages(struct page *page, struct page *buddy)
{
	struct page *left;

	free_page_del(page);
	free_page_del(buddy);

	page->order++;
	page->flags |= PAGE_FLAG_FREE;

	buddy->order++;
	buddy->flags |= PAGE_FLAG_FREE;

	left = page->paddr < buddy->paddr ? page : buddy;
	page_set_compound_head(left, left->order);

	return left;
}

static void merge_free_page(struct page *page)
{
	struct page *buddy;
	uint32_t order;

	for (order = page->order; order < (PAGE_MAX_ORDER - 1); order++) {
		buddy = page_buddy(page, page->order);

		/* Buddy page is currently in use */
		if (!page_is_free(buddy))
			return;

		/* Merge pages and get left buddy */
		page = merge_pages(page, buddy);

		/* Add left buddy to free area */
		free_page_add(page, order);
	}
}

static void __free_pages(struct page *page, uint32_t order)
{
	free_page_add(page, order);
	page->flags |= PAGE_FLAG_FREE;

	/* Merge page with buddy */
	merge_free_page(page);
}

static void __free_page(struct page *page)
{
	__free_pages(page, page->order);
}

void free_page(uint32_t addr)
{
	struct page *page = phys_to_page(addr);

	__free_page(page);
}

/*
 * Initialization
 */

static void compound_page_init(uint32_t paddr, uint32_t order)
{
	struct page *page;
	uint32_t pfn;

	for (pfn = 0; pfn < (1UL << order); pfn++) {
		page = phys_to_page(paddr + pfn * PAGE_SIZE);

		page->flags = PAGE_FLAG_FREE;
		page->paddr = paddr + pfn * PAGE_SIZE;
		page->compound_head = phys_to_page(paddr);
		list_init(&page->list);
		page->order = order;
		page->inuse = 0;
		list_init(&page->freelist);
	}
}

static void __page_map_free_bootmem(struct memblock_region *region)
{
	uint32_t addr, rest, align, order, offset;
	struct page *page;

	addr = region->base;
	rest = region->size;

	while (rest) {
		align = round_down_pow2(rest);

		if (align > PAGE_MAX_ALLOC_SIZE)
			align = PAGE_MAX_ALLOC_SIZE;

		order = log2(align) - PAGE_SHIFT;

		/* Get the correct order for the chunk */
		while (!page_is_aligned(addr, order))
			order--;

		/* Initialize all pages of the same se */
		compound_page_init(addr, order);

		/* Get compound page head */
		page = phys_to_page(addr);
		free_page_append(page, order);

		offset = (1UL << order) * PAGE_SIZE;

		rest -= offset;
		addr += offset;
	}
}

static void page_map_free_bootmem(void)
{
	struct memblock_region *region;
	uint32_t ridx;

	for_each_free_memblock(ridx, region) {

		/*
		 * For now we restrict the maximum range
		 * for allocations in the first megabyte
		 * since we don't want to accidentally
		 * override our future kernel in memory.
		 */

		if (memblock_end(region) > MEMORY_LIMIT)
			break;

		__page_map_free_bootmem(region);
	}
}

static int page_map_init(void)
{
	uint32_t order;

	page_num = MEMORY_LIMIT / PAGE_SIZE;

	/* Allocate page map array */
	page_map = memblock_alloc(page_num * sizeof(struct page), PAGE_SIZE);
	if (!page_map)
		return -ENOMEM;

	for (order = 0; order < PAGE_MAX_ORDER; order++) {
		list_init(&page_free_area_order(order)->list);
		page_free_area_order(order)->nr_free = 0;
	}

	return 0;
}

int page_alloc_init(void)
{
	if (page_map_init())
		return -EFAULT;

	page_map_free_bootmem();

	return 0;
}