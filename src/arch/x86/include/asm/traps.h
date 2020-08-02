#ifndef __X86_TRAPS_H__
#define __X86_TRAPS_H__

#define X86_STUB_ENTRY_SIZE		9

#define X86_TRAP_OFFSET			0
#define X86_TRAP_NUM			20
#define X86_TRAP_ENTRY_SIZE		X86_STUB_ENTRY_SIZE
#define X86_TRAP_ERROR_ENTRIES	X86_TRAP_NUM
#define X86_TRAP_ERROR_CODES	0x00227d00

#define X86_INTR_OFFSET			32
#define X86_INTR_NUM			16
#define X86_INTR_ENTRY_SIZE		X86_STUB_ENTRY_SIZE

#define X86_IDT_NUM_ENTRIES		256

#ifndef __ASSEMBLY__

/*
 * CPU state structure pt_regs
 */

struct pt_regs {
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;

	/* Segments */
	uint32_t ds;
	uint32_t es;
	uint32_t fs;
	uint32_t gs;

	/* Interrupt vector */
	uint32_t vector;

	/* Error code */
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
};

/*
 * Supported interrupts
 */

enum {
	X86_TRAP_DE			= 0x00,
	X86_TRAP_DB			= 0x01,
	X86_TRAP_NMI		= 0x02,
	X86_TRAP_BP			= 0x03,
	X86_TRAP_OF			= 0x04,
	X86_TRAP_BR			= 0x05,
	X86_TRAP_UD			= 0x06,
	X86_TRAP_NM			= 0x07,
	X86_TRAP_DF			= 0x08,
	X86_TRAP_OLD_MF		= 0x0A,
	X86_TRAP_TS			= 0x0B,
	X86_TRAP_NP			= 0x0C,
	X86_TRAP_SS			= 0x0D,
	X86_TRAP_GP			= 0x0E,
	X86_TRAP_PF			= 0x0F,
	X86_TRAP_MF			= 0x10,
	X86_TRAP_AC			= 0x11,
	X86_TRAP_MC			= 0x12,
	X86_TRAP_XF			= 0x13,
	X86_TRAP_VE			= 0x14
};

#endif /* __ASSEMBLY__ */

#endif /* __X86_TRAPS_H__ */