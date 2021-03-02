#ifndef __X86_BOOTPARAM_H__
#define __X86_BOOTPARAM_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <asm/video.h>

/*
 * E820 memory map structure
 */

#define E820_MAX_ENTRIES                          8
#define SMAP                                      0x534d4150

#define E820_MEMORY_TYPE_INVALID                   0
#define E820_MEMORY_TYPE_AVAILABLE                 1
#define E820_MEMORY_TYPE_RESERVED                  2
#define E820_MEMORY_TYPE_ACPI_RECLAIMABLE          3
#define E820_MEMORY_TYPE_ACPI_NVS                  4
#define E820_MEMORY_TYPE_BAD_MEMORY                5

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

/*
 * APM BIOS structure
 */

/*
 * Boot params structure:
 *
 * This structure contains all basic information about the underlying
 * machine as well as the capacities provided by this bootloader. The
 * boot_params structure needs to be filled in real mode as many from
 * those information can only be retrieved then.
 *
 * In stage three, the information from this structure are then taken
 * and adjusted to the multiboot information structure which can then
 * be read by the OS.
 */

struct boot_params {

	/*
	 * Boot information table
	 */
	struct boot_info_table *boot_table;

	/*
	 * Basic memory info
	 */
	uint32_t memory_lower;
	uint32_t memory_upper;

	/*
	 * BIOS boot device
	 */
	uint32_t disk_drive;

	/*
	 * Boot command line
	 */
	char *cmdline;

	/*
	 * E820 memory map
	 */
	struct e820_table e820_table;

	/*
	 * APM table
	 */
	// struct apm_info apm_info;

	/*
	 * VBE information
	 */
	struct vesa_info vesa_info;
	struct vesa_mode vesa_mode;
	uint16_t vesa_current_mode;
} __packed;

#endif /* __X86_BOOTPARAM_H__ */