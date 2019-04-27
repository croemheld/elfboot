#ifndef __X86_BOOT_H__
#define __X86_BOOT_H__

#define BOOT_IMAGE_ADDR                           0x7C00
#define BOOT_STACK_ADDR                           0x1800

#define SETUP_PHYS_ADDR                           0x8000
#define SETUP_SEGMENT                             0x07E0

#define VGA_FONT_BITMAP_ADDRESS                   0xA000
#define CMDLINE_BUFFER_ADDRESS                    0xB000

#define REALMODE_ADDRESS_LIMIT                    0xFFFF

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <uapi/asm/bootparam.h>
#include <uapi/asm/processor-flags.h>

#include <boot/asm.h>
#include <boot/bios.h>
#include <boot/cmdline.h>
#include <boot/disk.h>
#include <boot/edd.h>
#include <boot/elf.h>
#include <boot/elf32.h>
#include <boot/elf64.h>
#include <boot/iso.h>
#include <boot/printf.h>
#include <boot/regs.h>
#include <boot/segment.h>
#include <boot/string.h>
#include <boot/tty.h>
#include <boot/video.h>

#include <boot/linkage.h>

#define ARRAY_SIZE(x)                             (sizeof(x) / sizeof(*(x)))

/*
 * Conversions
 */

static inline void *uinttvptr(uint32_t val)
{
	return (void *)val;
}

static inline uint32_t vptrtuint(void *ptr)
{
	return (uint32_t)ptr;
}

static inline void *vptradd(void *ptr, uint32_t offset)
{
	return ptr + offset;
}

/* 
 * Basic port I/O 
 */

static inline void outb(uint8_t v, uint16_t port)
{
	__asm__ volatile("outb %0, %1" :: "a" (v), "dN" (port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t v;

	__asm__ volatile("inb %1, %0" : "=a" (v) : "dN" (port));

	return v;
}

static inline void outw(uint16_t v, uint16_t port)
{
	__asm__ volatile("outw %0, %1" :: "a" (v), "dN" (port));
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t v;

	__asm__ volatile("inw %1, %0" : "=a" (v) : "dN" (port));

	return v;
}

static inline void outl(uint32_t v, uint16_t port)
{
	__asm__ volatile("outl %0, %1" :: "a" (v), "dN" (port));
}

static inline uint32_t inl(uint16_t port)
{
	uint32_t v;

	__asm__ volatile("inl %1, %0" : "=a" (v) : "dN" (port));

	return v;
}

static inline void io_delay(void)
{
	const uint16_t delay_port = 0x80;

	__asm__ volatile("outb %%al, %0" :: "dN" (delay_port));
}

/* These functions are used to reference data in other segments. */

static inline uint16_t get_ds(void)
{
	uint16_t seg;

	__asm__ volatile("movw %%ds, %0" : "=rm" (seg));

	return seg;
}

static inline void set_fs(uint16_t seg)
{
	__asm__ volatile("movw %0, %%fs" :: "rm" (seg));
}

static inline uint16_t get_fs(void)
{
	uint16_t seg;

	__asm__ volatile("movw %%fs, %0" : "=rm" (seg));

	return seg;
}

static inline void set_gs(uint16_t seg)
{
	__asm__ volatile("movw %0, %%gs" :: "rm" (seg));
}

static inline uint16_t get_gs(void)
{
	uint16_t seg;

	__asm__ volatile("movw %%gs, %0" : "=rm" (seg));

	return seg;
}

/*
 * Number utility functions
 */

static inline int is_digit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static inline int is_xdigit(int ch)
{
	if (is_digit(ch))
		return true;

	if ((ch >= 'a') && (ch <= 'f'))
		return true;

	return (ch >= 'A') && (ch <= 'F');
}

/* Retrieve e820 memory map */
void detect_memory(struct boot_params *boot_params);

/* Detect supported video modes */
void detect_videos(struct boot_params *boot_params);

/* Load kernel */
void prepare_kernel(struct boot_params *boot_params);

/* Jump into our cr0S kernel */
void kernel_init(void);

#endif /* __ASSEMBLER__ */

#endif /* __X86_BOOT_H__ */