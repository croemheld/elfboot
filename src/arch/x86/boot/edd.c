#include <boot/boot.h>

/*
 * Disk Address Packet structure
 */

struct disk_address_packet dap = {
	.dap_size   = DAP_STRUCT_SIZE,
	.dap_secnum = DAP_BUFFER_SECTORS
};

/*
 * Pointer to PVD
 */

struct iso_pvd *pvd = NULL;

int edd_read_sector(uint8_t devno, uint16_t offset, uint32_t sector)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x42;
	ireg.dl = devno;
	ireg.si = vptrtuint(&dap);

	dap.dap_offset = offset;
	dap.dap_sector = sector;

	bioscall(0x13, &ireg, &oreg);

	if(oreg.eflags & X86_EFLAGS_CF)
		return 0;

	return 1;
}