#ifndef __X86_BOOT_H__
#define __X86_BOOT_H__

#define BOOT_IMAGE_ADDR                           0x7C00
#define BOOT_STACK_ADDR                           0x1800

#define SETUP_PHYS_ADDR                           0x8000
#define SETUP_SEGMENT                             0x07E0

#define VGA_FONT_BITMAP_ADDRESS                   0xA000
#define CMDLINE_BUFFER_ADDRESS                    0xB000

#define REALMODE_ADDRESS_LIMIT                    0xFFFF

#define IMG_ADDRESS                               0x7C00
#define IMG_MAX_ADDRESS                           0x00010000
#define IMG_MAX_SIZE                             (IMG_MAX_ADDRESS - IMG_ADDRESS)

#define IVT_ADDRESS                               0x0000
#define IVT_MAX_SIZE                              0x0400

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <asm/asm.h>
#include <asm/bios.h>
#include <asm/edd.h>
#include <asm/printf.h>
#include <asm/segment.h>
#include <asm/tty.h>
#include <asm/video.h>

#include <asm/linkage.h>

#include <uapi/asm/bootparam.h>
#include <uapi/asm/processor-flags.h>

#define ARRAY_SIZE(x)                             (sizeof(x) / sizeof(*(x)))

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