#ifndef __X86_IO_H__
#define __X86_IO_H__

#include <elfboot/core.h>

/* 
 * Basic port I/O 
 */

static inline void outb(uint16_t port, uint8_t v)
{
	__asm__ volatile("outb %0, %1" :: "a" (v), "dN" (port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t v;

	__asm__ volatile("inb %1, %0" : "=a" (v) : "dN" (port));

	return v;
}

static inline void outw(uint16_t port, uint16_t v)
{
	__asm__ volatile("outw %0, %1" :: "a" (v), "dN" (port));
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t v;

	__asm__ volatile("inw %1, %0" : "=a" (v) : "dN" (port));

	return v;
}

static inline void outl(uint16_t port, uint32_t v)
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

#endif /* __X86_IO_H__ */