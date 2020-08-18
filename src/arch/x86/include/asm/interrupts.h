#ifndef __X86_INTERRUPTS_H__
#define __X86_INTERRUPTS_H__

#include <asm/pic.h>
#include <asm/traps.h>

#define ARCH_NUM_INTERRUPTS		X86_IDT_NUM_ENTRIES

static inline void arch_enable_clock(void)
{
	pic_unmask_irq(X86_INTR_PIT);
}

static inline void arch_disable_clock(void)
{
	pic_mask_irq(X86_INTR_PIT);
}

static inline void arch_enable_keyboard(void)
{
	pic_unmask_irq(X86_INTR_KBD);
}

static inline void arch_disable_keyboard(void)
{
	pic_mask_irq(X86_INTR_KBD);
}

static inline void arch_suspend_machine(void)
{
	asm volatile("hlt");
}

#endif /* __X86_INTERRUPTS_H__ */