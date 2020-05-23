#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/linkage.h>

#define __ASSEMBLY__

#include <asm/traps.h>

.code32

.section .text

.macro PUSH_PT_REGS
	cld

	/* Segments */
	pushl   %gs
	pushl   %fs
	pushl   %es
	pushl   %ds

	/*
	 * We want to push the stack pointer but we also have to remember, that
	 * we already pushed 5 segment descriptors as well as that the CPU also
	 * pushed 3 or 4 dwords (eflags, cs, eip and if given, error_code). The
	 * fifth dword is pushed by us, namely the vector.
	 *
	 * To get the original ESP register value at the moment we received the
	 * interrupt, we have to add: 9 * sizeof(uint32_t) = 0x24 to the pushed
	 + ESP value.
	 */
	pushl   %esp
	addl    $0x24, (%esp)

	/* Registers */
	pushl   %edi
	pushl   %esi
	pushl   %ebp
	pushl   %ebx
	pushl   %edx
	pushl   %ecx
	pushl   %eax
.endm

.macro POP_PT_REGS

	/* Registers */
	popl    %eax
	popl    %ecx
	popl    %edx
	popl    %ebx
	popl    %ebp
	popl    %esi
	popl    %edi

	/* ESP, Segments, IRET */
	addl    $0x1C, %esp
.endm

/*
 * For our bootloader we only want a really simple and compact IDT which is
 * why we try to limit the instructions as well as the actually implemented
 * Interrupt Service Routines to a minimum.
 */

SYMBOL(exception_list, SYM_L_GLOBAL, 4)
	i = 0
	.rept X86_TRAP_NUM

	/*
	 * Check the error code bitmap if the bit is cleared for a specific ISR
	 * so that we can push a dummy value of zero on top of the stack to get
	 * the stack to be consistent.
	 */
	.if X86_TRAP_ERROR_ENTRIES > i && ((X86_TRAP_ERROR_CODES >> i) & 1) == 0
	pushl   $0
	.endif

	/*
	 * We also need to know, which interrupt was triggered. We achieve this
	 * by pushing the interrupt vector, which is represented by the iterate
	 * variable "i", on the stack.
	 */
	pushl   $i

	jmp     handle_interrupt

	i = i + 1
	.fill exception_list + i * X86_TRAP_ENTRY_SIZE - ., 1, 0xCC
	.endr
ENDPROC(exception_list)

SYMBOL(interrupt_list, SYM_L_GLOBAL, 4)
	i = 0
	.rept X86_INTR_NUM

	/*
	 * Similar to the exception list stubs we define another list dedicated
	 * for I/O interrupts caused by devices. We implement it similarily and
	 * push a dummy error_code value everytime.
	 */
	pushl   $0
	pushl   $(i + 32)

	jmp     handle_interrupt

	i = i + 1
	.fill interrupt_list + i * X86_INTR_ENTRY_SIZE - ., 1, 0xCC
	.endr
ENDPROC(interrupt_list)

LOCAL(handle_interrupt)
	PUSH_PT_REGS
	pushl   %esp

	calll   handle_generic_interrupt

	addl    $4, %esp
	POP_PT_REGS

	iret
ENDPROC(handle_interrupt)