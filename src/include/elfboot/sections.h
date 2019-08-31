#ifndef __ELFBOOT_SECTIONS_H__
#define __ELFBOOT_SECTIONS_H__

#include <elfboot/core.h>

#include <uapi/elfboot/common.h>

extern char __boot_start[], __boot_end[];
extern char __text_start[], __text_end[];
extern char __rodata_start[], __rodata_end[];
extern char __data_start[], __data_end[];
extern char __bss_start[], __bss_end[];

#define SECTION_START(section)	((uint32_t)__##section##_start)
#define SECTION_END(section)	((uint32_t)__##section##_end)

#define SECTION_SIZE(section)	(SECTION_END(section) - SECTION_START(section))

/*
 * Macros for global sections
 */

#define BOOT_START		(SECTION_START(boot))
#define BOOT_END		(SECTION_END(boot))

#define TEXT_START		(SECTION_START(text))
#define TEXT_END		(SECTION_END(text))

#define RODATA_START		(SECTION_START(rodata))
#define RODATA_END		(SECTION_END(rodata))

#define DATA_START		(SECTION_START(data))
#define DATA_END		(SECTION_END(data))

#define BSS_START		(SECTION_START(bss))
#define BSS_END			(SECTION_END(bss))

static inline bool is_elfboot_rodata(uint32_t addr)
{
	return (addr >= RODATA_START) && (addr < RODATA_END);
}

#endif /* __ELFBOOT_SECTIONS_H__ */