#include <asm/boot.h>
#include "bda.h"
#include "acpi.h"

static uint8_t acpi_rsdp_checksum(void *buffer, int length)
{
	uint8_t *end, sum;
	uint8_t *address = buffer;

	end = vptradd(address, length);
	sum = 0;

	while(address < end)
		sum += *(address++);

	return sum;
}

struct rsdp_descriptor *acpi_scan_rsdp_memory(uint32_t base, uint32_t length)
{
	struct rsdp_descriptor *rsdp;
	void *address, *end;

	end = uinttvptr(base + length);

	for(address = start; address < end; address = vptradd(address, 16)) {
		rsdp = address;

		if (strncmp(rsdp->signature, ACPI_RSDP_SIGNATURE, 8))
			continue;

		if (acpi_rsdp_checksum(rsdp, ACPI_RSDP_CHECKSUM_LENGTH))
			continue;

		if(rsdp->revision >= 2 
			&& acpi_rsdp_checksum(rsdp, ACPI_RSDP_XCHECKSUM_LENGTH))
			continue;

		return address;
	}

	return NULL;
}

struct rsdp_descriptor *acpi_get_rsdp(void)
{
	struct rsdp_descriptor *rsdp;
	uint32_t ebda_base_addr = bios_get_ebda_addr();

	rsdp = acpi_scan_rsdp_memory(ebda_base_addr, ACPI_EBDA_RANGE);

	if (rsdp)
		return rsdp;

	rsdp = acpi_scan_rsdp_memory(ACPI_HMEM_BASE, ACPI_HMEM_RANGE);

	if (rsdp)
		return rsdp;

	return NULL;
}