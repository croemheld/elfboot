#ifndef __EXT2_FS_H__
#define __EXT2_FS_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#define EXT2_SUPERBLOCK_OFFSET	1024
#define EXT2_SUPERBLOCK_LENGTH	1024

struct ext2_superblock {
	uint32_t inodes_count;
	uint32_t blocks_count;
	uint32_t reserved_blocks_count;
	uint32_t free_blocks_count;
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;
	uint32_t log_frag_size;
	uint32_t blocks_per_grpup;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;
	uint32_t wtime;
	uint16_t mnt_count;
	uint16_t max_mnt_count;
	uint16_t magic;
	uint16_t state;
	uint16_t errors;
	uint16_t revision_minor;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creatoros;
	uint32_t revision;
	uint16_t def_resuid;
	uint16_t def_resgid;
	uint8_t  unused[940];
} __packed;

struct ext2_block_group_desc {
	uint32_t block_usage_bitmap_block;
	uint32_t inode_usage_bitmap_block;
	uint32_t inode_table_block;
	uint16_t group_blocks_available;
	uint16_t group_inodes_available;
	uint16_t group_directories;
	uint8_t  unused[14];
} __packed;

#define EXT2_NUM_BLOCKS		12

struct ext2_inode {
	uint16_t type;

#define EXT2_PERM_OEX		0x0001
#define EXT2_PERM_OWR		0x0002
#define EXT2_PERM_ORD		0x0004
#define EXT2_PERM_GEX		0x0008
#define EXT2_PERM_GWR		0x0010
#define EXT2_PERM_GRD		0x0020
#define EXT2_PERM_UEX		0x0040
#define EXT2_PERM_UWR		0x0080
#define EXT2_PERM_URD		0x0100

#define EXT2_TYPE_SBIT		0x0200
#define EXT2_TYPE_SGID		0x0400
#define EXT2_TYPE_SUID		0x0800
#define EXT2_TYPE_FIFO		0x1000
#define EXT2_TYPE_CDEV		0x2000
#define EXT2_TYPE_DIR		0x4000
#define EXT2_TYPE_BDEV		0x6000
#define EXT2_TYPE_FILE		0x8000
#define EXT2_TYPE_SYM		0xA000
#define EXT2_TYPE_SOCK		0xC000

	uint16_t uid;
	uint32_t size_lower;
	uint32_t last_access;
	uint32_t ctime;
	uint32_t last_modify;
	uint32_t dtime;
	uint16_t gid;
	uint16_t hardlink_count;
	uint32_t sector_count;
	uint32_t flags;

#define EXT2_SECURE_DELETE	0x00000001
#define EXT2_COPY_DELETION	0x00000002
#define EXT2_FILE_COMPRESS	0x00000004
#define EXT2_SYNC_UPDATES	0x00000008
#define EXT2_IMMUTABLE_FILE	0x00000010
#define EXT2_APPEND_ONLY	0x00000020
#define EXT2_IN_DUMP		0x00000040
#define EXT2_UPDATE_LAST_ACCESS	0x00000080

#define EXT2_HASH_INDEXED_DIR	0x00010000
#define EXT2_AFS_DIR		0x00020000
#define EXT2_JOURNAL_FILE_DATA	0x00040000

	uint32_t osd1;
	uint32_t dbp[EXT2_NUM_BLOCKS];
	uint32_t singly_block;
	uint32_t doubly_block;
	uint32_t triply_block;
	uint32_t generation;
	uint32_t facl;
	union {
		uint32_t size_upper;
		uint32_t dacl;
	};
	uint32_t fragment_block;
	uint8_t  osd2[12];
} __packed;

struct ext2_directory {
	uint32_t inode;
	uint16_t size;
	uint8_t  name_length;
	uint8_t  type;

#define EXT2_DIR_UNKNOWN	0
#define EXT2_DIR_FILE		1
#define EXT2_DIR_DIRECTORY	2
#define EXT2_DIR_CHAR_DEVICE	3
#define EXT2_DIR_BLOCK_DEVICE	4
#define EXT2_DIR_FIFO		5
#define EXT2_DIR_SOCKET		6
#define EXT2_DIR_SYMLINK	7

	char name[0];
} __packed;

void ext2_fs_init(void);

#endif /* __EXT2_FS_H__ */