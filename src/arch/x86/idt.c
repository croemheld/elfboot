#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/interrupts.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/ptrace.h>
#include <asm/traps.h>
#include <asm/desc.h>
#include <asm/pic.h>

#include <uapi/elfboot/common.h>

#define TYPE_ATTR(type, dpl)	(1 << 7) | ((dpl) << 5) | (type)

#define G(__vector, __addr, __type, __dpl, __segment)	\
	{													\
		.offset_lower	= (__addr),						\
		.segment		= (__segment),					\
		.__reserved		= 0,							\
		.type_attribute	= TYPE_ATTR((__type), (__dpl)),	\
		.offset_upper	= 0								\
	}

#define INTG(__vector, __addr)						\
	G(__vector, __addr, GATE_INTR, 0, GDT_CODE32)

/*
 * Interrupt entry points
 */

extern const char exception_list[X86_TRAP_NUM][X86_TRAP_ENTRY_SIZE];

extern const char interrupt_list[X86_INTR_NUM][X86_INTR_ENTRY_SIZE];

/*
 * Interrupt Descriptor Table
 */

static struct gate_desc *idt = NULL;

static struct desc_ptr idt_desc;

static void set_interrupt_gate(uint32_t vector, const void *addr)
{
	struct gate_desc desc = INTG(vector, tuint(addr));

	memcpy(&idt[vector], &desc, sizeof(desc));
}

void handle_generic_interrupt(struct pt_regs *regs)
{
	if (has_interrupt_handler(regs->vector)) {
		interrupt_callback(regs->vector, regs);
	} else {
#ifdef CONFIG_X86_INTERRUPT_INFO
		bprintln("INTR: Unknown interrupt %lu (%lx) at RIP: %08lx",
			regs->vector, regs->error_code,  regs->eip);
		bprintln("INTR: eax: %08lx, ebx: %08lx, ecx: %08lx, edx: %08lx",
			regs->eax, regs->ebx, regs->ecx, regs->edx);
		bprintln("INTR: esp: %08lx, ebp: %08lx, esi: %08lx, edi: %08lx",
			regs->esp, regs->ebp, regs->esi, regs->edi);

		__dump_stack(regs->eip, regs->ebp);
#endif
	}

	pic_send_eoi(regs->vector);
}

int arch_init_interrupts(void)
{
	uint32_t vector, len;

	len = X86_IDT_NUM_ENTRIES * sizeof(struct gate_desc);
	idt = bzalloc(len);
	if (!idt)
		return -ENOMEM;

	bprintln("IDT: Setting IDT (%lx)...", &idt_desc);

	idt_desc.limit  = len - 1;
	idt_desc.offset = tuint(idt);

	asm volatile("lidt (%0)" :: "r" (&idt_desc));

	pic_init();

	for (vector = 0; vector < X86_TRAP_NUM; vector++)
		set_interrupt_gate(vector + X86_TRAP_OFFSET, exception_list[vector]);

	for (vector = 0; vector < X86_INTR_NUM; vector++)
		set_interrupt_gate(vector + X86_INTR_OFFSET, interrupt_list[vector]);

	asm volatile("sti");

	return 0;
}

