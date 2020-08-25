#ifndef __ELFBOOT_INTERRUPT_H__
#define __ELFBOOT_INTERRUPT_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <asm/interrupts.h>

#define NUM_INTERRUPTS		ARCH_NUM_INTERRUPTS

struct interrupt_handler {
	const char *name;
	void (*callback)(void *info);
	struct list_head list;
};
/*
 * Keyboard - Enable & disable input
 */

void enable_keyboard(void);

void disable_keyboard(void);

/*
 * Clock - Enable & disable clock
 */

void enable_clock(void);

void disable_clock(void);

/*
 * Interrupt handler registration
 */

void register_interrupt_handler(uint32_t vector,
	struct interrupt_handler *handler);

void unregister_interrupt_handler(struct interrupt_handler *handler);

bool has_interrupt_handler(uint32_t vector);

void interrupt_callback(uint32_t vector, void *info);

int init_interrupts(void);

#endif /* __ELFBOOT_INTERRUPT_H__ */