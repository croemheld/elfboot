#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/interrupts.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

struct list_head *interrupt_handlers = NULL;
static uint32_t num_interrupts = 0;

void register_interrupt_handler(uint32_t vector,
	struct interrupt_handler *handler)
{
	if (vector >= num_interrupts)
		return;

	list_add(&handler->list, &interrupt_handlers[vector]);
}

bool has_interrupt_handler(uint32_t vector)
{
	return !list_empty(&interrupt_handlers[vector]);
}

void interrupt_callback(uint32_t vector)
{
	struct interrupt_handler *handler;

	list_for_each_entry(handler, &interrupt_handlers[vector], list) {
		if (handler->callback) {
			bprintln("INTERRUPT: Invoke %s callback...", handler->name);

			handler->callback();
		}
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