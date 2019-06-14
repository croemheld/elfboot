#include <elfboot/core.h>

#include <asm/bda.h>

#include <uapi/elfboot/common.h>

struct bios_data_area *bios_get_bda(void)
{
	return uinttvptr(BDA_ADDRESS);
}

uint32_t bios_get_ebda_addr(void)
{
	uint32_t bseg, ebda;

	bseg = bios_get_bda()->ebda_segment;
	ebda = segment_offset_addr(bseg, 0);

	if (ebda < EBDA_LOWMEM_START || ebda > EBDA_LOWMEM_LIMIT)
		return -1;

	return ebda;
}

void *bios_get_ebda_ptr(void)
{
	uint32_t ebda_addr = bios_get_ebda_addr();

	if (ebda_addr == -1UL)
		return NULL;

	return uinttvptr(ebda_addr);
}