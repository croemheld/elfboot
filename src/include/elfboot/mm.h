#ifndef __ELFBOOT_MM_H__
#define __ELFBOOT_MM_H__

#include <elfboot/core.h>
#include <elfboot/page.h>
#include <elfboot/slub.h>
#include <elfboot/memblock.h>

#include <uapi/elfboot/const.h>

#define MEMORY_START			0x010000
#define MEMORY_LIMIT			0x100000

void bfree(void *dptr);

void *bmalloc(size_t size);

void *bzalloc(size_t size);

char *bstrdup(const char *str);

void bfree_const(const void *ptr);

void *brealloc(void *dptr, size_t size);

int bmalloc_init(void);

/*
 * Page allocation
 */

struct page *alloc_pages(uint32_t order);

struct page *alloc_page(void);

static inline uint32_t page_alloc(uint32_t order)
{
	struct page *page = alloc_pages(order);

	return (uint32_t)page_address(page);
}

void *get_zeroed_page(void);

void free_page(uint32_t addr);

int page_alloc_init(void);

#endif /* __ELFBOOT_MM_H__ */