#include <asm/edd.h>

static void edd_read_legacy_params(uint8_t devno, struct edd_device_info *edi)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x08;
	ireg.dl = devno;

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return;

	edi->legacy_max_cylinder = oreg.ch + ((oreg.cl & 0xc0) << 2);
	edi->legacy_max_head = oreg.dh;
	edi->legacy_sectors_per_track = oreg.cl & 0x3f;
}

static int edd_extensions_present(uint8_t devno, struct edd_device_info *edi)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x41;
	ireg.bx = EDD_MAGIC1;
	ireg.dl = devno;

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return oreg.ah;

	if (oreg.bx != EDD_MAGIC2)
		return -1;

	edi->device = devno;
	edi->version = oreg.ah;
	edi->interface_support = oreg.cx;

	return 0;
}

static int edd_read_device_params(uint8_t devno, struct edd_device_params *edp)
{
	struct biosregs ireg, oreg;
	struct edd_device_params ebuf;

	initregs(&ireg);
	ireg.ah = 0x48;
	ireg.dl = devno;
	ireg.si = vptrtuint(&ebuf);

	ebuf.length = sizeof(*edp);

	bioscall(0x13, &ireg, &oreg);

	if (oreg.eflags & X86_EFLAGS_CF)
		return oreg.ah;

	memcpy(edp, &ebuf, sizeof(*edp));

	return 0;
}

int edd_read_device_info(uint8_t devno, struct edd_device_info *edi)
{
	/*
	 * Reading legacy parameters.
	 *
	 * edd_read_legacy_params may fail depending on the underlying 
	 * device. However, the initialization should not be aborted 
	 * even if it fails. It just means the current drive does not 
	 * support old CHS addressing.
	 */
	
	edd_read_legacy_params(devno, edi);

	/*
	 * Reading extended drive parameters.
	 *
	 * If the current device does not support LBA addressing, we 
	 * simply abort the initialization procedure since the device 
	 * must then support CHS addressing.
	 */

	if (edd_extensions_present(devno, edi))
		return -1;

	if (edd_read_device_params(devno, &edi->params))
		return -1;

	return 0;
}