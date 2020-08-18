#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/interrupts.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

struct list_head *interrupt_handlers = NULL;
static uint32_t num_interrupts = 0;

/*
 * Keyboard - Enable & disable input
 */

void enable_keyboard(void)
{
	arch_enable_keyboard();
}

void disable_keyboard(void)
{
	arch_disable_keyboard();
}

/*
 * Clock - Enable & disable clock
 */

void enable_clock(void)
{
	arch_enable_clock();
}

void disable_clock(void)
{
	arch_disable_clock();
}

/*
 * Interrupt handler registration
 */

void register_interrupt_handler(uint32_t vector,
	struct interrupt_handler *handler)
{
	if (vector >= num_interrupts)
		return;

	list_add(&handler->list, &interrupt_handlers[vector]);
}

void unregister_interrupt_handler(struct interrupt_handler *handler)
{
	list_del(&handler->list);
}

bool has_interrupt_handler(uint32_t vector)
{
	return !list_empty(&interrupt_handlers[vector]);
}

void interrupt_callback(uint32_t vector, void *info)
{
	struct interrupt_handler *handler;

	list_for_each_entry(handler, &interrupt_handlers[vector], list) {
		if (!handler->callback)
			return;

		handler->callback(info);
	}
}

int init_interrupts(void)
{
	int i;

	interrupt_handlers = bmalloc(sizeof(*interrupt_handlers) * NUM_INTERRUPTS);
	if (!interrupt_handlers)
		return -ENOMEM;

	num_interrupts = NUM_INTERRUPTS;

	for (i = 0; i < NUM_INTERRUPTS; i++)
		list_init(&interrupt_handlers[i]);

	return 0;
}