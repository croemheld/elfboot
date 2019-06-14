#ifndef __X86_E820_H__
#define __X86_E820_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#define E820_MAX_ENTRIES                          16
#define SMAP                                      0x534d4150

/*
 * E820 memory map structure
 */

struct e820_entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
} __packed;

struct e820_table {
	uint16_t nr_entries;
	struct e820_entry entries[E820_MAX_ENTRIES];
} __packed;

#define E820_MEMORY_TYPE_INVALID                   0
#define E820_MEMORY_TYPE_AVAILABLE                 1
#define E820_MEMORY_TYPE_RESERVED                  2
#define E820_MEMORY_TYPE_ACPI_RECLAIMABLE          3
#define E820_MEMORY_TYPE_ACPI_NVS                  4
#define E820_MEMORY_TYPE_BAD_MEMORY                5

#endif /* __X86_E820_H__ */