#ifndef __X86_DISK_H__
#define __X86_DISK_H__

/* Address where the PVD is loaded */

#define PVD_BUFFER_SEGMENT                        0x0200
#define PVD_BUFFER_ADDRESS                        (PVD_BUFFER_SEGMENT << 4)

/* Buffer address for temporary directories when searching for files */

#define DIR_BUFFER_SEGMENT                        0x0280
#define DIR_BUFFER_ADDRESS                        (DIR_BUFFER_SEGMENT << 4)

#define DAT_BUFFER_SEGMENT                        0x0300
#define DAT_BUFFER_ADDRESS                        (DAT_BUFFER_SEGMENT << 4)

#define TMP_BUFFER_SEGMENT                        0x0400
#define TMP_BUFFER_ADDRESS                        (DAT_BUFFER_SEGMENT << 4)

#define DIR_BUFFER_SIZE                           0x0800
#define DAT_BUFFER_SIZE                           0x1000

#define DAT_BLOCK_FRAGMENT                        (DAT_BUFFER_SIZE >> 2)
#define DAT_BUFFER_SECTORS                        (DAT_BUFFER_SIZE >> 11)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/*
 * Disk Address Packet (DAP)
 */

struct disk_address_packet {
	uint8_t  dap_size;
	uint8_t  _reserved;
	uint16_t dap_secnum;
	uint16_t dap_offset;
	uint16_t dap_segment;
	uint32_t dap_sector, dap_sector_msb;
} __attribute__((packed));

#define DAP_STRUCT_SIZE                       sizeof(struct disk_address_packet)
#define DAP_BUFFER_SECTORS                        DAT_BUFFER_SECTORS

#endif /* __X86_DISK_H__ */