#ifndef __X86_BOOT_H__
#define __X86_BOOT_H__

#define BOOT_IMAGE_ADDR                           0x7C00
#define BOOT_STACK_ADDR                           0x1800

#define SETUP_PHYS_ADDR                           0x8000
#define SETUP_SEGMENT                             0x07E0

#define CMDLINE_BUFFER_ADDRESS 0xA000

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
#include <boot/disk.h>
#include <boot/video.h>
#include <boot/segment.h>
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

/*
 * Utility functions
 */

void initregs(struct biosregs *reg);

void memory_dump(struct boot_params *boot_params);

/*
 * String utility functions
 */

void *memcpy(void *dst, const void *src, size_t len);

void *memset(void *dst, int c, size_t len);

void farcpy(void *dst, const void *src, size_t len);

int memcmp(const void *s1, const void *s2, size_t len);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t count);

size_t strnlen(const char *str, size_t maxlen);

size_t strlen(const char *str);

char *strstr(const char *str1, const char *str2);

char *strchr(const char *str, int c);

uint32_t simple_strtoull(const char *cp, char **endp, unsigned int base);

int simple_strtol(const char *cp, char **endp, unsigned int base);

/*
 * TTY
 */

void putc(int ch);

void puts(const char *str);

/*
 * Print
 */

int vsnprintf(char *buffer, size_t size, const char *format, va_list *argp);

int snprintf(char *buffer, size_t size, const char *format, ...);

int sprintf(char* buffer, const char *format, ...);

int bprintf(const char *format, ...);

/*
 * EDD
 */

int edd_read_sector(uint8_t devno, uint16_t offset, uint32_t sector);

/*
 * ISO 9660
 */

void iso_print_records(uint8_t devno, struct iso_dir *parent);

int iso_load_file(uint8_t devno, uint32_t offset, const char *path);

/*
 * Command line
 */

bool cmdline_get_boolean_value(const char *key);

bool cmdline_get_boolean_default_value(const char *key, bool default_value);

uint32_t cmdline_get_int_value(const char *key);

uint32_t cmdline_get_int_default_value(const char *key, uint32_t default_value);

char *cmdline_get_string_value(const char *key);

char *cmdline_get_string_default_value(const char *key, const char *default_value);

/*
 * Boot functions
 */

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