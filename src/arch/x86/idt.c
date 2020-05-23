#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
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
		.offset_lower	= __addr,						\
		.segment		= __segment,					\
		.type_attribute	= TYPE_ATTR(__type, __dpl),		\
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

static struct gate_desc idt_table[X86_IDT_NUM_ENTRIES] = { 0 };

static struct desc_ptr idt_desc;

static void set_intr_gate(uint32_t vector, const void *addr)
{
	struct gate_desc desc = INTG(vector, tuint(addr));

	memcpy(&idt_table[vector], &desc, sizeof(desc));
}

void handle_generic_interrupt(struct pt_regs *regs)
{
	bprintln("INTR: Unknown interrupt %lu with error code %lx at RIP: %08lx",
		regs->vector, regs->error_code, regs->eip);

	bprintln("INTR: Registers:");

	bprintln("INTR: eax: %08lx, ebx: %08lx, ecx: %08lx, edx: %08lx",
		regs->eax, regs->ebx, regs->ecx, regs->edx);

	bprintln("INTR: esp: %08lx, ebp: %08lx, esi: %08lx, edi: %08lx",
		regs->esp, regs->ebp, regs->esi, regs->edi);

	dump_stack(regs->eip, regs->ebp);

	pic_send_eoi(regs->vector);
}

int arch_init_interrupts(void)
{
	uint32_t i;

	pic_init();

	for (i = 0; i < X86_TRAP_NUM; i++)
		set_intr_gate(i, exception_list[i]);

	for (i = 0; i < X86_INTR_NUM; i++)
		set_intr_gate(i + 0x20, interrupt_list[i]);

	idt_desc.size = X86_IDT_NUM_ENTRIES * sizeof(struct gate_desc) - 1;
	idt_desc.addr_lower = tuint(idt_table);

	bprintln("IDT: Setting IDT (%lx)...", idt_table);

	asm volatile("lidt %0" :: "m" (idt_desc));

	return 0;
}

