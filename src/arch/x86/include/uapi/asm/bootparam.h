#ifndef __BOOTPARAM_H__
#define __BOOTPARAM_H__

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define E820_MAX_ENTRIES                          16
#define SMAP                                      0x534d4150

/*
 * E820 memory map structure
 */

struct e820_entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
} __attribute__((packed));

/*
 * Boot params structure
 */

struct boot_params {
	uint16_t disk_drive;
	uint16_t e820_count;
	uint32_t code_entry;
	struct e820_entry e820_table[E820_MAX_ENTRIES];
} __attribute__((packed));

#endif /* __ASSEMBLER__ */

#endif /* __BOOTPARAM_H__ */