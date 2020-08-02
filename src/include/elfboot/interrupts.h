#ifndef __ELFBOOT_INTERRUPT_H__
#define __ELFBOOT_INTERRUPT_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <asm/interrupts.h>

#define NUM_INTERRUPTS		ARCH_NUM_INTERRUPTS

struct interrupt_handler {
	const char *name;
	void (*callback)(void);
	struct list_head list;
};

void register_interrupt_handler(uint32_t vector,
	struct interrupt_handler *handler);

bool has_interrupt_handler(uint32_t vector);

void interrupt_callback(uint32_t vector);

int init_interrupts(void);

#endif /* __ELFBOOT_INTERRUPT_H__ */