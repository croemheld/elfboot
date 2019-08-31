#include <elfboot/core.h>
#include <elfboot/string.h>

#include <asm/bda.h>
#include <asm/acpi.h>

#include <uapi/elfboot/common.h>

static uint8_t acpi_rsdp_checksum(void *buffer, int length)
{
	uint8_t *end, sum;
	uint8_t *addr = buffer;

	end = vptradd(addr, length);
	sum = 0;

	while(addr < end)
		sum += *(addr++);

	return sum;
}

struct rsdp_descriptor *acpi_scan_rsdp_memory(uint32_t base, uint32_t length)
{
	struct rsdp_descriptor *rsdp;
	void *addr, *end;

	end  = uinttvptr(base + length);

	for(addr = uinttvptr(base); addr < end; addr = vptradd(addr, 16)) {
		rsdp = addr;

		if (strncmp(rsdp->signature, ACPI_RSDP_SIGNATURE, 8))
			continue;

		if (acpi_rsdp_checksum(rsdp, ACPI_RSDP_CHECKSUM_LENGTH))
			continue;

		if(rsdp->revision >= 2 
			&& acpi_rsdp_checksum(rsdp, ACPI_RSDP_XCHECKSUM_LENGTH))
			continue;

		return addr;
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