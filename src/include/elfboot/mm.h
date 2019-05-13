#ifndef __ELFBOOT_MM_H__
#define __ELFBOOT_MM_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <elfboot/e820.h>

void *bmalloc(size_t size);

void *bzalloc(size_t size);

char *bstrdup(const char *str);

void bfree(void *dptr);

void *brealloc(void *dptr, size_t size);

void bmalloc_init(struct e820_entry *e820_table, uint16_t e820_count);

#endif /* __ELFBOOT_MM_H__ */