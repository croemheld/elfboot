#ifndef __FS_ISO_H__
#define __FS_ISO_H__

#include <elfboot/core.h>

#include <uapi/elfboot/const.h>

#define ISO_SECTOR_SHIFT                          11
#define ISO_SECTOR_SIZE                           _BITUL(ISO_SECTOR_SHIFT)

#define ISO_DIR_FLAG_HIDDEN_BIT                    0
#define ISO_DIR_FLAG_HIDDEN                       _BITUL(ISO_DIR_FLAG_HIDDEN_BIT)
#define ISO_DIR_FLAG_DIRECTORY_BIT                 1
#define ISO_DIR_FLAG_DIRECTORY                    _BITUL(ISO_DIR_FLAG_DIRECTORY_BIT)
#define ISO_DIR_FLAG_ASSOCIATED_BIT                2
#define ISO_DIR_FLAG_ASSOCIATED                   _BITUL(ISO_DIR_FLAG_ASSOCIATED_BIT)
#define ISO_DIR_FLAG_FORMAT_BIT                    3
#define ISO_DIR_FLAG_FORMAT                       _BITUL(ISO_DIR_FLAG_FORMAT_BIT)
#define ISO_DIR_FLAG_PERMISSIONS_BIT               4
#define ISO_DIR_FLAG_PERMISSIONS                  _BITUL(ISO_DIR_FLAG_PERMISSIONS_BIT)
#define ISO_DIR_FLAG_MULTIPLE_EXT_BIT              7
#define ISO_DIR_FLAG_MULTIPLE_EXT                 _BITUL(ISO_DIR_FLAG_MULTIPLE_EXT_BIT)

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

/*
 * ISO- directory
 */

struct iso9660_dir_date {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t timezone;
} __attribute__((packed));

struct iso9660_dir {
	uint8_t length;
	uint8_t attr_lenght;
	uint32_t extent;
	uint32_t extent_msb;
	uint32_t size;
	uint32_t size_msb;
	struct iso9660_dir_date date;
	uint8_t flags;
	uint16_t interleave;
	uint32_t volume_sequence_number;
	uint8_t name_length;
	char name[0];
} __attribute__((packed));

/*
 * Primary Volume Descriptor
 */

struct iso9660_pvd_date {
	char year[4];
	char month[2];
	char day[2];
	char hour[2];
	char minute[2];
	char second[2];
	char hseconds[2];
	uint8_t timezone;
} __attribute__((packed));

struct iso9660_pvd {
	uint8_t type;
	char id[5];
	uint8_t version;
	uint8_t _reserved1;
	char system_id[32];
	char volume_id[32];
	uint64_t _reserved2;
	uint32_t volume_size;
	uint32_t volume_size_msb;
	char _reserved3[32];
	uint16_t volume_set_size;
	uint16_t volume_set_size_msb;
	uint16_t volume_sequence_number;
	uint16_t volume_sequence_number_msb;
	uint16_t logical_block_size;
	uint16_t logical_block_size_msb;
	uint32_t path_table_size;
	uint32_t path_table_size_msb;
	uint32_t path_table_l_extent;
	uint32_t opt_path_table_l_extent;
	uint32_t path_table_m_extent;
	uint32_t opt_path_table_m_extent;
	struct iso9660_dir root_directory;
	char volume_set_id[128];
	char publisher_id[128];
	char preparer_id[128];
	char application_id[128];
	char copyright_file_id[38];
	char abstract_file_id[36];
	char bibliographic_file_id[37];
	struct iso9660_pvd_date creation_date;
	struct iso9660_pvd_date modification_date;
	struct iso9660_pvd_date expiration_date;
	struct iso9660_pvd_date effective_date;
	uint8_t fs_version;
	uint8_t _reserved4;
	char application_data[512];
	char _reserved5[653];
} __attribute__((packed));

void iso_print_records(uint8_t devno, struct iso9660_dir *parent);

int iso_load_file(uint8_t devno, uint32_t offset, const char *path);

#endif /* __FS_ISO_H__ */