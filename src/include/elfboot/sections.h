#ifndef __ELFBOOT_SECTIONS_H__
#define __ELFBOOT_SECTIONS_H__

#include <elfboot/core.h>

extern char __boot_start[], __boot_end[];
extern char __text_start[], __text_end[];
extern char __rodata_start[], __rodata_end[];
extern char __data_start[], __data_end[];
extern char __bss_start[], __bss_end[];

#define SECTION_START(section)	((uint32_t)__##section##_start)
#define SECTION_END(section)	((uint32_t)__##section##_end)

#define SECTION_SIZE(section)	(SECTION_END(section) - SECTION_START(section))

#endif /* __ELFBOOT_SECTIONS_H__ */