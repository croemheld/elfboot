#ifndef __ELFBOOT_MM_H__
#define __ELFBOOT_MM_H__

#include <elfboot/core.h>

#include <uapi/elfboot/const.h>

#define PAGE_SHIFT		12
#define PAGE_SIZE		_BITUL(PAGE_SHIFT)
#define PAGE_MASK		(PAGE_SIZE - 1)
#define PAGE_ADDRESS(x)		((x) & ~(PAGE_MASK))

void *bmalloc(size_t size);

void *bzalloc(size_t size);

char *bstrdup(const char *str);

void bfree(void *dptr);

void *brealloc(void *dptr, size_t size);

void bmalloc_init(void);

#endif /* __ELFBOOT_MM_H__ */