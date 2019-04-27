#include <asm/boot.h>
#include "bda.h"

struct bios_data_area *bios_get_bda(void)
{
	return uinttvptr(BDA_ADDRESS);
}

uint32_t bios_get_ebda_addr(void)
{
	uint32_t bseg, ebda;

	bseg = bios_get_bda()->ebda_segment;
	ebda = segment_offset_addr(bseg, 0);

	if (ebda < (LOWMEM_LIMIT - EBDA_MAX_SIZE) || ebda > LOWMEM_LIMIT)
		return -1;

	return ebda;
}

void *bios_get_ebda_ptr(void)
{
	uint32_t ebda_addr = bios_get_ebda_addr();

	if (ebda_addr == -1)
		return NULL;

	return uinttvptr(ebda_addr);
}