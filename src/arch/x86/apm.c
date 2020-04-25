#include <asm/boot.h>
#include <asm/bios.h>

#include <uapi/asm/bootparam.h>
#include <uapi/asm/processor-flags.h>

int apm_installation_check(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x53;

	bioscall(0x15, &ireg, &oreg);

	/* APM not supported */
	if (oreg.flags & X86_EFLAGS_CF)
		return -EFAULT;

	/* APM "PM" signature */
	if (oreg.bx != APM_SIGNATURE)
		return -EFAULT;

	/* 32-bit not suppoerted */
	if (!(oreg.cx & 0x02))
		return -EFAULT;

	/* Disconnect first, just in case */
	ireg.al = 0x04;
	intcall(0x15, &ireg, NULL);

	/* 32-bit connect */
	ireg.al = 0x03;
	intcall(0x15, &ireg, &oreg);
}