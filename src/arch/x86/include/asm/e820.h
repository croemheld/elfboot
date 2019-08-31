#ifndef __X86_E820_H__
#define __X86_E820_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

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
	union {
		struct {
			uint32_t addr_32;
			uint32_t addr_reserved;
		};
		uint64_t addr_64;
	};
	union {
		struct {
			uint32_t size_32;
			uint32_t size_reserved;
		};
		uint64_t size_64;
	};
	uint32_t type;
} __packed;

struct e820_table {
	uint16_t nr_entries;
	struct e820_entry entries[E820_MAX_ENTRIES];
} __packed;

#endif /* __X86_E820_H__ */