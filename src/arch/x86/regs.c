#include <asm/boot.h>
#include "regs.h"

/*
 * Initialize registers for bioscalls
 */

void initregs(struct biosregs *reg)
{
	/*
	 * The caller is responsible for initialising 
	 * every other member of the biosregs structure.
	 */

	memset(reg, 0, sizeof(*reg));
	reg->eflags |= X86_EFLAGS_CF;
}