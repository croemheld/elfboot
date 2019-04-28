#ifndef __IO_ATA_H__
#define __IO_ATA_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "disk.h"
#include "device.h"

/*
 * ATA status registers
 */

#define ATA_SR_BSY                                0x80
#define ATA_SR_DRDY                               0x40
#define ATA_SR_DF                                 0x20
#define ATA_SR_DSC                                0x10
#define ATA_SR_DRQ                                0x08
#define ATA_SR_CORR                               0x04
#define ATA_SR_IDX                                0x02
#define ATA_SR_ERR                                0x01

/*
 * ATA error registers
 */

#define ATA_ER_BBK                                0x80
#define ATA_ER_UNC                                0x40
#define ATA_ER_MC                                 0x20
#define ATA_ER_IDNF                               0x10
#define ATA_ER_MCR                                0x08
#define ATA_ER_ABRT                               0x04
#define ATA_ER_TK0NF                              0x02
#define ATA_ER_AMNF                               0x01

/*
 * ATA commands
 */

#define ATA_CMD_READ_PIO                          0x20
#define ATA_CMD_READ_PIO_EXT                      0x24
#define ATA_CMD_READ_DMA                          0xC8
#define ATA_CMD_READ_DMA_EXT                      0x25
#define ATA_CMD_WRITE_PIO                         0x30
#define ATA_CMD_WRITE_PIO_EXT                     0x34
#define ATA_CMD_WRITE_DMA                         0xCA
#define ATA_CMD_WRITE_DMA_EXT                     0x35
#define ATA_CMD_CACHE_FLUSH                       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT                   0xEA
#define ATA_CMD_PACKET                            0xA0
#define ATA_CMD_IDENTIFY_PACKET                   0xA1
#define ATA_CMD_IDENTIFY                          0xEC

#define ATAPI_CMD_READ                            0xA8
#define ATAPI_CMD_EJECT                           0x1B

#define ATA_IDENT_DEVICETYPE                        0
#define ATA_IDENT_CYLINDERS                         2
#define ATA_IDENT_HEADS                             6
#define ATA_IDENT_SECTORS                          12
#define ATA_IDENT_SERIAL                           20
#define ATA_IDENT_MODEL                            54
#define ATA_IDENT_CAPABILITIES                     98
#define ATA_IDENT_FIELDVALID                      106
#define ATA_IDENT_MAX_LBA                         120
#define ATA_IDENT_COMMANDSETS                     164
#define ATA_IDENT_MAX_LBA_EXT                     200

#define IDE_ATA                                   0x00
#define IDE_ATAPI                                 0x01
 
#define ATA_MASTER                                0x00
#define ATA_SLAVE                                 0x01

/*
 * Additional ATA registers
 */

#define ATA_REG_DATA                               0
#define ATA_REG_ERROR                              1
#define ATA_REG_FEATURES                           1
#define ATA_REG_SECTORS                            2
#define ATAPI_REG_IREASON                          2
#define ATA_REG_LBA0                               3
#define ATA_REG_LBA1                               4
#define ATAPI_REG_CNTLOW                           4
#define ATA_REG_LBA2                               5
#define ATAPI_REG_CNTHIGH                          5
#define ATA_REG_HDDEVSEL                           6
#define ATA_REG_COMMAND                            7
#define ATA_REG_STATUS                             7
#define ATA_REG_SECCOUNT1                          8
#define ATA_REG_LBA3                               9
#define ATA_REG_LBA4                              10
#define ATA_REG_LBA5                              11
#define ATA_REG_CONTROL                           12
#define ATA_REG_ALTSTATUS                         12
#define ATA_REG_DEVADDRESS                        13

#define ATAPI_IREASON_MASK                        0x3
#define ATAPI_IREASON_DATA_OUT                    0x0
#define ATAPI_IREASON_CMD_OUT                     0x1
#define ATAPI_IREASON_DATA_IN                     0x2
#define ATAPI_IREASON_ERROR                       0x3

/*
 * Channels
 */

#define ATA_PRIMARY                               0x00
#define ATA_SECONDARY                             0x01

/*
 * Directions
 */

#define ATA_READ                                  0x00
#define ATA_WRITE                                 0x01

/*
 * Logical Block Addressing
 */

#define ATA_LBA_PIO28_LIMIT                       0xFFFFFFFF

/*
 * Misc
 */

#define ATA_IO_TIMEOUT                            100000
#define ATA_IDENTIFY_BUFFER_SIZE                  512
#define ATA_IDENTIFY_WORD_COUNT                   (ATA_IDENTIFY_BUFFER_SIZE / 2)

struct ata_scsi_packet {
	union {
		uint8_t raw[16];
	};
};

struct ata_regs {
	union {
		uint8_t raw[11];
		struct {
			union {
				uint8_t features;
				uint8_t error;
			};
			union {
				uint8_t sectors;
				uint8_t atapi_ireason;
			};
			union {
				uint8_t lba_low;
				uint8_t sectnum;
			};
			union {
				uint8_t lba_mid;
				uint8_t cyllsb;
				uint8_t atapi_cntlow;
			};
			union {
				uint8_t lba_high;
				uint8_t cylmsb;
				uint8_t atapi_cnthigh;
			};
			uint8_t disk;
			union {
				uint8_t cmd;
				uint8_t status;
			};
			uint8_t sectors48;
			uint8_t lba48_low;
			uint8_t lba48_mid;
			uint8_t lba48_high;
		};

	};
} __attribute__((packed));

struct ata_params {
	void *buf;
	int bufsize;
	void *cmd;
	int cmdsize;
	int write;
	struct ata_regs regs;
} __attribute__((packed));

static inline uint64_t chs_to_lba(uint64_t heads, uint64_t sectors, 
				  uint64_t c, uint64_t h, uint64_t s)
{
	return (c * heads + h) * sectors + (s - 1);
}

static inline uint32_t lba_to_chs_cylinder(uint64_t lba, uint32_t hpc, 
					   uint32_t spt)
{
	return lba / (hpc * spt);
}

static inline uint32_t lba_to_chs_head(uint64_t lba, uint32_t hpc, 
				       uint32_t spt)
{
	return (lba % (hpc * spt)) / spt;
}

static inline uint32_t lba_to_chs_sector(uint64_t lba, uint32_t hpc, 
					 uint32_t spt)
{
	return (lba % (hpc * spt)) % spt + 1;
}

#define lba_to_cyln(lba, hpc, spt)	\
	lba_to_chs_cylinder(lba, hpc, spt)

#define lba_to_head(lba, hpc, spt)	\
	lba_to_chs_head(lba, hpc, spt)

#define lba_to_sect(lba, hpc, spt)	\
	lba_to_chs_sector(lba, hpc, spt)

int ata_read_sectors(struct device *device, void *buffer, uint64_t lba,
		     int sectnum);

#define ata_read_sector(d, b, l)                  ata_read_sectors(d, b, l, 1)

void ata_firmware_init(void);

#endif /* __IO_ATA_H__ */