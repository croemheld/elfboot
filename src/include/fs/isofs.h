#ifndef __ISOFS_FS_H__
#define __ISOFS_FS_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <uapi/elfboot/const.h>

#define ISO_SECTOR_SHIFT		11
#define ISO_SECTOR_SIZE			_BITUL(ISO_SECTOR_SHIFT)

#define ISO_DIR_HIDDEN_BIT		0
#define ISO_DIR_HIDDEN			_BITUL(ISO_DIR_HIDDEN_BIT)
#define ISO_DIR_DIRECTORY_BIT		1
#define ISO_DIR_DIRECTORY		_BITUL(ISO_DIR_DIRECTORY_BIT)
#define ISO_DIR_ASSOCIATED_BIT		2
#define ISO_DIR_ASSOCIATED		_BITUL(ISO_DIR_ASSOCIATED_BIT)
#define ISO_DIR_FORMAT_BIT		3
#define ISO_DIR_FORMAT			_BITUL(ISO_DIR_FORMAT_BIT)
#define ISO_DIR_PERMISSIONS_BIT		4
#define ISO_DIR_PERMISSIONS		_BITUL(ISO_DIR_PERMISSIONS_BIT)
#define ISO_DIR_MULTIPLE_EXT_BIT	7
#define ISO_DIR_MULTIPLE_EXT		_BITUL(ISO_DIR_MULTIPLE_EXT_BIT)

/* Offsets within the PVD structure */

#define PVD_TYPE_CODE_OFFSET			  0
#define PVD_STD_ID_OFFSET			  1
#define PVD_VERSNO_OFFSET			  6
#define PVD_SYS_ID_OFFSET			  8
#define PVD_VOL_ID_OFFSET			 40
#define PVD_SPACE_SIZE_OFFSET			 80
#define PVD_SET_SIZE_OFFSET			120
#define PVD_SEQ_NUMBER_OFFSET			124
#define PVD_BLOCK_SIZE_OFFSET			128
#define PVD_PATH_TABLE_SIZE_OFFSET		132
#define PVD_PATH_TABLE_L_OFFSET			140
#define PVD_OPT_PATH_TABLE_L_OFFSET		144
#define PVD_PATH_TABLE_M_OFFSET			148
#define PVD_OPT_PATH_TABLE_M_OFFSET		152
#define PVD_ROOT_DIRECTORY_OFFSET		156
#define PVD_VOL_SET_ID_OFFSET			190
#define PVD_PUB_ID_OFFSET			318
#define PVD_DATA_PREP_ID_OFFSET			446
#define PVD_APPLICATION_ID_OFFSET		574
#define PVD_COPYRIGHT_ID_OFFSET			702
#define PVD_ABSTRACT_ID_OFFSET			740
#define PVD_BIBLIOGRAPHIC_ID_OFFSET		776
#define PVD_VOL_CREATE_DATE_OFFSET		813
#define PVD_VOL_MODIFY_DATE_OFFSET		830
#define PVD_VOL_EXPIRE_DATE_OFFSET		847
#define PVD_VOL_EFFECT_DATE_OFFSET		864
#define PVD_FILE_STRUCTURE_VERSNO_OFFSET	881

/* Constants */

#define PVD_TYPE_BR				  0
#define PVD_TYPE_PVD				  1
#define PVD_TYPE_SVD				  2
#define PVD_TYPE_VPD				  3

/* ISO 9660 directory fields */

#define DIR_LENGTH_OFFSET			  0
#define DIR_EXT_ATTR_LENGTH_OFFSET		  1
#define DIR_EXTR_SEC_OFFSET			  2
#define DIR_DATA_LEN_OFFSET			 10
#define DIR_DATE_OFFSET				 18
#define DIR_FILE_FLAG_OFFSET			 25
#define DIR_FILE_UNIT_SIZE_OFFSET		 26
#define DIR_FILE_INTL_SIZE_OFFSET		 27
#define DIR_VSEQ_NUM_OFFSET			 28
#define DIR_FILE_ID_LENGTH_OFFSET		 32
#define DIR_FILE_ID_OFFSET			 33

/*
 * ISO directory
 */

struct isofs_dir_date {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t timezone;
} __packed;

struct isofs_dir {
	uint8_t length;
	uint8_t attr_lenght;
	uint32_t extent;
	uint32_t extent_msb;
	uint32_t size;
	uint32_t size_msb;
	struct isofs_dir_date date;
	uint8_t flags;
	uint16_t interleave;
	uint32_t volume_sequence_number;
	uint8_t name_length;
	char name[0];
} __packed;

/*
 * Primary Volume Descriptor
 */

struct isofs_pvd_date {
	char year[4];
	char month[2];
	char day[2];
	char hour[2];
	char minute[2];
	char second[2];
	char hseconds[2];
	uint8_t timezone;
} __packed;

struct isofs_pvd {
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
	struct isofs_dir root_directory;
	char volume_set_id[128];
	char publisher_id[128];
	char preparer_id[128];
	char application_id[128];
	char copyright_file_id[38];
	char abstract_file_id[36];
	char bibliographic_file_id[37];
	struct isofs_pvd_date creation_date;
	struct isofs_pvd_date modification_date;
	struct isofs_pvd_date expiration_date;
	struct isofs_pvd_date effective_date;
	uint8_t fs_version;
	uint8_t _reserved4;
	char application_data[512];
	char _reserved5[653];
} __packed;

/* -------------------------------------------------------------------------- */

/*
 * Borrowing from linux and ultimately from bsd386.
 */

#define ISODCL(from, to)	(to - from + 1)

static inline int isonum_711(uint8_t *p)
{
	return *p;
}

static inline int isonum_712(int8_t *p)
{
	return *p;
}

static inline unsigned int isonum_721(uint8_t *p)
{
	return get_le16(p);
}

static inline unsigned int isonum_722(uint8_t *p)
{
	return get_be16(p);
}

static inline unsigned int isonum_723(uint8_t *p)
{
	return get_le16(p);
}

static inline unsigned int isonum_731(uint8_t *p)
{
	return get_le32(p);
}

static inline unsigned int isonum_732(uint8_t *p)
{
	return get_be32(p);
}

static inline unsigned int isonum_733(uint8_t *p)
{
	return get_le32(p);
}

struct iso_volume_descriptor {
	uint8_t type			[ISODCL (   1,    1)]; /* 711 */
	char id				[ISODCL (   2,    6)];
	uint8_t version			[ISODCL (   7,    7)];
	uint8_t data			[ISODCL (   8, 2048)];
} __packed;

struct iso_primary_descriptor {
	uint8_t type			[ISODCL (   1,    1)]; /* 711 */
	char id				[ISODCL (   2,    6)];
	uint8_t version			[ISODCL (   7,    7)]; /* 711 */
	uint8_t unused1			[ISODCL (   8,    8)];
	char system_id			[ISODCL (   9,   40)]; /* achars */
	char volume_id			[ISODCL (  41,   72)]; /* dchars */
	uint8_t unused2			[ISODCL (  73,   80)];
	uint8_t volume_space_size	[ISODCL (  81,   88)]; /* 733 */
	uint8_t unused3			[ISODCL (  89,  120)];
	uint8_t volume_set_size		[ISODCL ( 121,  124)]; /* 723 */
	uint8_t volume_sequence_number	[ISODCL ( 125,  128)]; /* 723 */
	uint8_t logical_block_size	[ISODCL ( 129,  132)]; /* 723 */
	uint8_t path_table_size		[ISODCL ( 133,  140)]; /* 733 */
	uint8_t type_l_path_table	[ISODCL ( 141,  144)]; /* 731 */
	uint8_t opt_type_l_path_table	[ISODCL ( 145,  148)]; /* 731 */
	uint8_t type_m_path_table	[ISODCL ( 149,  152)]; /* 732 */
	uint8_t opt_type_m_path_table	[ISODCL ( 153,  156)]; /* 732 */
	uint8_t root_directory_record	[ISODCL ( 157,  190)]; /* 9.1 */
	char volume_set_id		[ISODCL ( 191,  318)]; /* dchars */
	char publisher_id		[ISODCL ( 319,  446)]; /* achars */
	char preparer_id		[ISODCL ( 447,  574)]; /* achars */
	char application_id		[ISODCL ( 575,  702)]; /* achars */
	char copyright_file_id		[ISODCL ( 703,  739)]; /* 7.5 dchars */
	char abstract_file_id		[ISODCL ( 740,  776)]; /* 7.5 dchars */
	char bibliographic_file_id	[ISODCL ( 777,  813)]; /* 7.5 dchars */
	uint8_t creation_date		[ISODCL ( 814,  830)]; /* 8.4.26.1 */
	uint8_t modification_date	[ISODCL ( 831,  847)]; /* 8.4.26.1 */
	uint8_t expiration_date		[ISODCL ( 848,  864)]; /* 8.4.26.1 */
	uint8_t effective_date		[ISODCL ( 865,  881)]; /* 8.4.26.1 */
	uint8_t file_structure_version	[ISODCL ( 882,  882)]; /* 711 */
	uint8_t unused4			[ISODCL ( 883,  883)];
	uint8_t application_data	[ISODCL ( 884, 1395)];
	uint8_t unused5			[ISODCL (1396, 2048)];
} __packed;

struct iso_supplementary_descriptor {
	uint8_t type			[ISODCL (   1,    1)]; /* 711 */
	char id				[ISODCL (   2,    6)];
	uint8_t version			[ISODCL (   7,    7)]; /* 711 */
	uint8_t flags			[ISODCL (   8,    8)]; /* 853 */
	char system_id			[ISODCL (   9,   40)]; /* achars */
	char volume_id			[ISODCL (  41,   72)]; /* dchars */
	uint8_t unused2			[ISODCL (  73,   80)];
	uint8_t volume_space_size	[ISODCL (  81,   88)]; /* 733 */
	uint8_t escape			[ISODCL (  89,  120)]; /* 856 */
	uint8_t volume_set_size		[ISODCL ( 121,  124)]; /* 723 */
	uint8_t volume_sequence_number	[ISODCL ( 125,  128)]; /* 723 */
	uint8_t logical_block_size	[ISODCL ( 129,  132)]; /* 723 */
	uint8_t path_table_size		[ISODCL ( 133,  140)]; /* 733 */
	uint8_t type_l_path_table	[ISODCL ( 141,  144)]; /* 731 */
	uint8_t opt_type_l_path_table	[ISODCL ( 145,  148)]; /* 731 */
	uint8_t type_m_path_table	[ISODCL ( 149,  152)]; /* 732 */
	uint8_t opt_type_m_path_table	[ISODCL ( 153,  156)]; /* 732 */
	uint8_t root_directory_record	[ISODCL ( 157,  190)]; /* 9.1 */
	char volume_set_id		[ISODCL ( 191,  318)]; /* dchars */
	char publisher_id		[ISODCL ( 319,  446)]; /* achars */
	char preparer_id		[ISODCL ( 447,  574)]; /* achars */
	char application_id		[ISODCL ( 575,  702)]; /* achars */
	char copyright_file_id		[ISODCL ( 703,  739)]; /* 7.5 dchars */
	char abstract_file_id		[ISODCL ( 740,  776)]; /* 7.5 dchars */
	char bibliographic_file_id	[ISODCL ( 777,  813)]; /* 7.5 dchars */
	uint8_t creation_date		[ISODCL ( 814,  830)]; /* 8.4.26.1 */
	uint8_t modification_date	[ISODCL ( 831,  847)]; /* 8.4.26.1 */
	uint8_t expiration_date		[ISODCL ( 848,  864)]; /* 8.4.26.1 */
	uint8_t effective_date		[ISODCL ( 865,  881)]; /* 8.4.26.1 */
	uint8_t file_structure_version	[ISODCL ( 882,  882)]; /* 711 */
	uint8_t unused4			[ISODCL ( 883,  883)];
	uint8_t application_data	[ISODCL ( 884, 1395)];
	uint8_t unused5			[ISODCL (1396, 2048)];
} __packed;

void isofs_fs_init(void);

#endif /* __ISOFS_FS_H__ */