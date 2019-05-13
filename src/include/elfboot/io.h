#ifndef __ELFBOOT_IO_H__
#define __ELFBOOT_IO_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

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

#endif /* __ELFBOOT_IO_H__ */