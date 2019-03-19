#ifndef __ISO_H__
#define __ISO_H__

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <uapi/cr0S/const.h>

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

/*
 * ISO- directory
 */

struct iso_dir_date {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t timezone;
} __attribute__((packed));

struct iso_dir {
	uint8_t length;
	uint8_t attr_lenght;
	uint32_t extent;
	uint32_t extent_msb;
	uint32_t size;
	uint32_t size_msb;
	struct iso_dir_date date;
	uint8_t flags;
	uint16_t interleave;
	uint32_t volume_sequence_number;
	uint8_t name_length;
	char name[0];
} __attribute__((packed));

/*
 * Primary Volume Descriptor
 */

struct iso_pvd_date {
	char year[4];
	char month[2];
	char day[2];
	char hour[2];
	char minute[2];
	char second[2];
	char hseconds[2];
	uint8_t timezone;
} __attribute__((packed));

struct iso_pvd {
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
	struct iso_dir root_directory;
	char volume_set_id[128];
	char publisher_id[128];
	char preparer_id[128];
	char application_id[128];
	char copyright_file_id[38];
	char abstract_file_id[36];
	char bibliographic_file_id[37];
	struct iso_pvd_date creation_date;
	struct iso_pvd_date modification_date;
	struct iso_pvd_date expiration_date;
	struct iso_pvd_date effective_date;
	uint8_t fs_version;
	uint8_t _reserved4;
	char application_data[512];
	char _reserved5[653];
} __attribute__((packed));

#endif /* __ASSEMBLER__ */

#endif /* __ISO_H__ */