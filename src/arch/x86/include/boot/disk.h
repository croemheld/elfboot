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

/* Offsets within the PVD structure */

#define PVD_TYPE_CODE_OFFSET                        0
#define PVD_STD_ID_OFFSET                           1
#define PVD_VERSNO_OFFSET                           6
#define PVD_SYS_ID_OFFSET                           8
#define PVD_VOL_ID_OFFSET                          40
#define PVD_SPACE_SIZE_OFFSET                      80
#define PVD_SET_SIZE_OFFSET                       120
#define PVD_SEQ_NUMBER_OFFSET                     124
#define PVD_BLOCK_SIZE_OFFSET                     128
#define PVD_PATH_TABLE_SIZE_OFFSET                132
#define PVD_PATH_TABLE_L_OFFSET                   140
#define PVD_OPT_PATH_TABLE_L_OFFSET               144
#define PVD_PATH_TABLE_M_OFFSET                   148
#define PVD_OPT_PATH_TABLE_M_OFFSET               152
#define PVD_ROOT_DIRECTORY_OFFSET                 156
#define PVD_VOL_SET_ID_OFFSET                     190
#define PVD_PUB_ID_OFFSET                         318
#define PVD_DATA_PREP_ID_OFFSET                   446
#define PVD_APPLICATION_ID_OFFSET                 574
#define PVD_COPYRIGHT_ID_OFFSET                   702
#define PVD_ABSTRACT_ID_OFFSET                    740
#define PVD_BIBLIOGRAPHIC_ID_OFFSET               776
#define PVD_VOL_CREATE_DATE_OFFSET                813
#define PVD_VOL_MODIFY_DATE_OFFSET                830
#define PVD_VOL_EXPIRE_DATE_OFFSET                847
#define PVD_VOL_EFFECT_DATE_OFFSET                864
#define PVD_FILE_STRUCTURE_VERSNO_OFFSET          881

/* Constants */

#define PVD_TYPE_BR                                 0
#define PVD_TYPE_PVD                                1
#define PVD_TYPE_SVD                                2
#define PVD_TYPE_VPD                                3

/* ISO 9660 directory fields */

#define DIR_LENGTH_OFFSET                           0
#define DIR_EXT_ATTR_LENGTH_OFFSET                  1
#define DIR_EXTR_SEC_OFFSET                         2
#define DIR_DATA_LEN_OFFSET                        10
#define DIR_DATE_OFFSET                            18
#define DIR_FILE_FLAG_OFFSET                       25
#define DIR_FILE_UNIT_SIZE_OFFSET                  26
#define DIR_FILE_INTL_SIZE_OFFSET                  27
#define DIR_VSEQ_NUM_OFFSET                        28
#define DIR_FILE_ID_LENGTH_OFFSET                  32
#define DIR_FILE_ID_OFFSET                         33

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <boot/iso.h>

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

#endif /* __ASSEMBLER__ */

#endif /* __X86_DISK_H__ */