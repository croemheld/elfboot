#ifndef __ELFBOOT_E820_H__
#define __ELFBOOT_E820_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define E820_MAX_ENTRIES                          16
#define SMAP                                      0x534d4150

#define E820_MEMORY_TYPE_INVALID                   0
#define E820_MEMORY_TYPE_AVAILABLE                 1
#define E820_MEMORY_TYPE_RESERVED                  2
#define E820_MEMORY_TYPE_ACPI_RECLAIMABLE          3
#define E820_MEMORY_TYPE_ACPI_NVS                  4
#define E820_MEMORY_TYPE_BAD_MEMORY                5

/*
 * E820 memory map structure
 */

struct e820_entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
} __attribute__((packed));

#endif /* __ELFBOOT_E820_H__ */