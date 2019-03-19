#include <boot/boot.h>

/*
 * Initialize registers for bioscalls
 */

void initregs(struct biosregs *reg)
{
	memset(reg, 0, sizeof(*reg));
	reg->eflags |= X86_EFLAGS_CF;
	reg->ds = get_ds();
	reg->es = get_ds();
	reg->fs = get_fs();
	reg->gs = get_gs();
}