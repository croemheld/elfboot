#ifndef __X86_BOOT_H__
#define __X86_BOOT_H__

#define ELFBOOT "ELFBOOT"

/*
 * Second stage information
 */

#define SECOND_STAGE_SEGMENT	0x07E0
#define SECOND_STAGE_ADDRESS	(SECOND_STAGE_SEGMENT << 4)

/*
 * elfboot logo information
 */

#define ELFBOOT_LOGO_SEGMENT	0x00000600
#define ELFBOOT_LOGO_ADDRESS	(ELFBOOT_LOGO_SEGMENT << 4)
#define ELFBOOT_LOGO_NUMWORD	0x00001000
#define VGA_FRBUFFER_ADDRESS	0x000b8000

/*
 * Boot stacks
 */

#define BOOT_STACK_START		0x6000
#define BOOT_STACK_END			0x5000
#define BOOT_STACK_SIZE			(BOOT_STACK_START - BOOT_STACK_END)

/*
 * Real-mode interrupt vector table
 */

#define IVT_ADDRESS                               0x0000
#define IVT_MAX_SIZE                              0x0400

#ifndef __ASSEMBLER__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <uapi/elfboot/common.h>

#include <uapi/asm/bootparam.h>

/*
 * Boot information table
 */

struct boot_info_table {
	uint32_t pvdlba;

	/*
	 * elfboot binary information
	 */
	uint32_t elfboot_lba;
	uint32_t elfboot_len;

	/*
	 * Boot unique identification file
	 */
	uint32_t bootuid_lba;
	uint32_t bootuid_len;
} __packed;

/*
 * Initialization
 */

int detect_memory(struct boot_params *boot_params);

int detect_videos(struct boot_params *boot_params);

/*
 * Common main function and arch_init_late
 */

int arch_init_interrupts(void);

int arch_init_late(void);

int elfboot_main(void);

#endif /* __ASSEMBLER__ */

#endif /* __X86_BOOT_H__ */