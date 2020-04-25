#ifndef __ELFBOOT_PAGE_H__
#define __ELFBOOT_PAGE_H__

#include <elfboot/core.h>
#include <elfboot/math.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

#define PAGE_SHIFT			12
#define PAGE_SIZE			_BITUL(PAGE_SHIFT)

struct free_area {
	uint32_t nr_free;
	struct list_head list;
};

struct page {
	uint32_t flags;

#define PAGE_FLAG_FREE_BIT		0
#define PAGE_FLAG_FREE			_BITUL(PAGE_FLAG_FREE_BIT)

	uint32_t paddr;
	uint32_t order;

	/*
	 * The list is used to insert the page to the freelist
	 * when the page is a compound head for a set of pages
	 * and is free for allocation.
	 *
	 * When the page is allocated, it is removed from the
	 * freelist and in the case of the slab allocator, it
	 * gets added to a list of slabs for a dedicated slab
	 * cache.
	 */

	struct list_head list;
	struct page *compound_head;

	/*
	 * In case this page is allocated for a slab cache we
	 * use the following members to store information for
	 * the dedicated slab cache in them.
	 */
	
	struct bmem_cache *slab_cache;
	uint32_t inuse;
	struct list_head freelist;
};

extern struct page *page_map;

static inline bool page_is_free(struct page *page)
{
	return (page->flags & PAGE_FLAG_FREE) != 0;
}

static inline uint32_t phys_to_pfn(uint32_t paddr)
{
	return paddr >> PAGE_SHIFT;
}

static inline uint32_t page_to_pfn(struct page *page)
{
	return phys_to_pfn(page->paddr);
}

static inline struct page *pfn_to_page(uint32_t pfn)
{
	return page_map + pfn;
}

static inline struct page *phys_to_page(uint32_t addr)
{
	return pfn_to_page(addr >> PAGE_SHIFT);
}

static inline void *page_address(struct page *page)
{
	return tvptr(page->paddr);
}

static inline struct page *compound_head(struct page *page)
{
	return page->compound_head;
}

static inline struct page *page_buddy(struct page *page, uint32_t order)
{
	uint32_t diff, bpfn, ppfn;

	/*
	 * We retrieve the buddy to a corresponding page by
	 * calculating the difference in their PFN. This only
	 * works because we allocated all pages in a continuous
	 * struct page array.
	 */

	ppfn = page_to_pfn(page);
	bpfn = phys_to_pfn(page->paddr ^ ((1ULL << order) * PAGE_SIZE));

	diff = abs_difference(bpfn, ppfn);

	if (ppfn < bpfn)
		return page + diff;

	return page - diff;
}

#endif /* __ELFBOOT_PAGE_H__ */