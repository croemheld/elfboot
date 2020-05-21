#include <elfboot/core.h>
#include <elfboot/libata.h>

static inline uint64_t libata_read(uint16_t *params, uint32_t word, uint64_t size)
{
	uint64_t i, value = 0;

	for (i = 0; i < size; i++) {

		/*
		 * Argument 'size' defines the number of words to read from the buffer
		 * which means that an 8-byte value has to be read in chunks 4 times.
		 */
		value |= params[word + i] << (16 * i);
	}

	return value;
}

bool libata_has_lba_support(uint16_t *params)
{
	return !!(libata_read(params, 49, 1) & ATA_SUPPORT_LBA);
}

bool libata_has_lba48_support(uint16_t *params)
{
	return !!(libata_read(params, 83, 1) & ATA_SUPPORT_ADDRESS48);
}

/*
 * libata_cylinders, libata_heads, libata_sectors_per_track: CHS addressing
 *
 * Every ATA device supports these functions. We only use them if the device
 * does not support LBA addressing mode. For LBA, we use the other dedicated
 * functions below.
 */
uint32_t libata_cylinders(uint16_t *params)
{
	return libata_read(params, 1, 1);
}

uint32_t libata_heads(uint16_t *params)
{
	return libata_read(params, 3, 1);
}

uint32_t libata_sectors_per_track(uint16_t *params)
{
	return libata_read(params, 6, 1);
}

/*
 * libata_last_block: Get the capacity of a device.
 *
 * Only works if the device supports LBA addressing. if the device does ot, we
 * have to use the legacy CHS addressing mode functions above.
 */
uint32_t libata_last_block(uint16_t *params)
{
	uint32_t last_block;

	if (!libata_has_lba48_support(params))
		last_block = libata_read(params,  60, 2);
	else
		last_block = libata_read(params, 100, 4);

	return last_block;
}

uint32_t libata_block_size(uint16_t *params)
{
	uint32_t block_size;

	if (libata_read(params, 106, 1) & ATA_PSS_VALID_VALUE) {
		block_size = libata_read(params, 117, 2);
		if (ATA_INVALID_BLOCK_SIZE(block_size))
			block_size = ATA_BLOCK_SIZE;
	} else
		block_size = ATA_BLOCK_SIZE;

	return block_size;
}