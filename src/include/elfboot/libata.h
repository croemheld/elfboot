#ifndef __ELFBOOT_LIBATA_H__
#define __ELFBOOT_LIBATA_H__

#include <elfboot/core.h>

#define ATA_BLOCK_WORDS			256
#define ATA_BLOCK_SIZE			(ATA_BLOCK_WORDS * 2)

#define ATA_IDENTIFY_SIZE		ATA_BLOCK_SIZE

#define ATA_SUPPORT_LBA			0x0200
#define ATA_SUPPORT_ADDRESS48	0x0400

#define ATA_PSS_VALID_MASK		0xC000
#define ATA_PSS_VALID_VALUE		0x4000

#define ATA_INVALID_BLOCK_SIZE(x)	((x) & ((x) - 1) || !(x) || (x) > 0x100000)

bool libata_has_lba_support(uint16_t *params);

bool libata_has_lba48_support(uint16_t *params);

/*
 * libata_cylinders, libata_heads, libata_sectors_per_track: CHS addressing
 *
 * Every ATA device supports these functions. We only use them if the device
 * does not support LBA addressing mode. For LBA, we use the other dedicated
 * functions below.
 */
uint32_t libata_cylinders(uint16_t *params);

uint32_t libata_heads(uint16_t *params);

uint32_t libata_sectors_per_track(uint16_t *params);

/*
 * libata_last_block: Get the capacity of a device.
 *
 * Only works if the device supports LBA addressing. if the device does ot, we
 * have to use the legacy CHS addressing mode functions above.
 */
uint32_t libata_last_block(uint16_t *params);

uint32_t libata_block_size(uint16_t *params);

#endif /* __ELFBOOT_LIBATA_H__ */