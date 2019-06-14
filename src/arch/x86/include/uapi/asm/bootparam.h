#ifndef __X86_BOOTPARAM_H__
#define __X86_BOOTPARAM_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <asm/e820.h>

/*
 * Boot params structure
 */

struct boot_params {
	uint16_t disk_drive;
	uint32_t code_entry;
	struct e820_table e820_table;
} __packed;

#endif /* __X86_BOOTPARAM_H__ */