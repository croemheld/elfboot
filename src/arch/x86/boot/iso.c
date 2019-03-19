#include <boot/boot.h>

#define CURRENT_ENTRY_NAME                        "."
#define UPLEVEL_ENTRY_NAME                        ".."

#define DRECORD_FLAGS_FILE                        "vdaEp  m"

#define DIRENTRY_SEPARATOR                        '/'

/*
 * Global variables
 */

struct iso_pvd *pvd;
int pvd_loaded = 0;

/*
 * ISO utility functions
 */

static void *iso_read_sector(uint8_t devno, uint32_t offset, uint32_t sector)
{
	if(!edd_read_sector(devno, offset, sector))
		return NULL;

	return uinttvptr(offset);
}

static void *iso_read_record(uint8_t devno, uint32_t offset, struct iso_dir *record)
{
	return iso_read_sector(devno, offset, record->extent);
}

static void *iso_read_directory(uint8_t devno, struct iso_dir *record)
{
	return iso_read_record(devno, DIR_BUFFER_ADDRESS, record);
}

static int iso_present_record(struct iso_dir *record)
{
	return record->length && record->name_length;
}

static struct iso_dir *iso_next_record(struct iso_dir *record)
{
	return vptradd(record, record->length);
}

static int iso_compare_id(struct iso_dir *record, const char *name, int length)
{
	return ((record->name_length == length) 
		&& !strncmp(record->name, name, length));
}

uint32_t iso_sector_length(struct iso_dir *record)
{
	return ((record->size + ISO_SECTOR_SIZE - 1) >> ISO_SECTOR_SHIFT);
}

/*
 * Debug
 */

static void iso_print_record_flags(struct iso_dir *record)
{
	uint8_t flags = record->flags;

	for(int i = 0; i < 8; i++) {
		if(DRECORD_FLAGS_FILE[i] == ' ')
			continue;

		if(flags & (1 << i))
			putc(DRECORD_FLAGS_FILE[i]);
		else
			putc('-');
	}
}

void iso_print_record(struct iso_dir *record)
{
	char name[32];

	/* Temporary length of identifier */
	int length = record->name_length;

	/* The stack may contain garbage values from older prints */
	memset(name, 0, ARRAY_SIZE(name));

	if(length == 1) {
		if(*record->name == 0) {
			memcpy(name, CURRENT_ENTRY_NAME, length);
		} else {
			length = 2;
			memcpy(name, UPLEVEL_ENTRY_NAME, length);
		}
	} else 
		memcpy(name, record->name, length);

	iso_print_record_flags(record);

	bprintf(" | %8d | %.*s\n", record->size, length, name);
}

void iso_print_records(uint8_t devno, struct iso_dir *parent)
{
	struct iso_dir *record;

	bprintf("Reading directory: %.*s...\n", parent->name_length, parent->name);

	record = iso_read_record(devno, TMP_BUFFER_ADDRESS, parent);

	if(!record) {
		bprintf("Could not read directory record %.*s\n", 
			parent->name_length, parent->name);
		return;
	}

	bprintf("%6s | %8s | %s\n", "Flags", "Size", "Filename");

	for(; iso_present_record(record); record = iso_next_record(record))
		iso_print_record(record);
}

/*
 * Lookup functions
 */

static int iso_validate_path(const char *path)
{
	int plen = strlen(path);

	if(plen == 1)
		return *path == DIRENTRY_SEPARATOR;

	return (path[0] == DIRENTRY_SEPARATOR)
		&& (path[plen - 1] != DIRENTRY_SEPARATOR);
}

static int iso_path_length(const char *path)
{
	char *fnptr;

	fnptr = strchr(path, DIRENTRY_SEPARATOR);

	if(!fnptr)
		return strlen(path);
	else
		return fnptr - path;
}

static int iso_load_pvd(uint8_t devno)
{
	uint16_t sector = 0x10;

	if(pvd_loaded)
		return 1;

	do {
		pvd = iso_read_sector(devno, PVD_BUFFER_ADDRESS, sector);

		if(pvd) {
			pvd_loaded = 1;
			return 1;
		}
		
		sector++;
	} while(1);
}

static struct iso_dir *iso_load_root_directory(uint8_t devno)
{
	if(!iso_load_pvd(devno))
		return 0;

	return iso_read_directory(devno, &pvd->root_directory);
}

static struct iso_dir *iso_lookup_record(struct iso_dir *record, 
	const char *path, int len)
{
	while(iso_present_record(record)) {
		if(!iso_compare_id(record, path, len)) {
			record = iso_next_record(record);
			continue;
		}

		return record;
	}

	return NULL;
}	

struct iso_dir *iso_lookup_path(uint8_t devno, const char *path)
{
	int fidlen;
	struct iso_dir *record;

	record = iso_load_root_directory(devno);

	if(!record)
		return NULL;

	if(!iso_validate_path(path))
		return NULL;

	if(strlen(path) == 1)
		return record;

	path++;

	while(iso_present_record(record)) {
		fidlen = iso_path_length(path);
		record = iso_lookup_record(record, path, fidlen);

		/* Last part of the path */
		if(strlen(path) == fidlen)
			return record;

		record = iso_read_directory(devno, record);

		if(!record)
			return NULL;

		path += (fidlen + 1);
	}

	return NULL;
}

int iso_load_file(uint8_t devno, uint32_t offset, const char *path)
{
	struct iso_dir *record;
	void *dst, *src;
	uint32_t sector, seclen, scount = 0;

	record = iso_lookup_path(devno, path);

	if(!record)
		return 0;

	dst = uinttvptr(offset);
	sector = record->extent;
	seclen = iso_sector_length(record);

	while(scount < seclen) {
		src = iso_read_sector(devno, DAT_BUFFER_ADDRESS, sector);

		if(!src)
			return 0;

		/*
		 * For performance reasons we only use memcpy when the
		 * destination is within the address range provided by
		 * the real mode segments.
		 */

		if(offset > REALMODE_ADDRESS_LIMIT)
			farcpy(dst, src, DAT_BUFFER_SIZE);
		else
			memcpy(dst, src, DAT_BUFFER_SIZE);

		dst += DAT_BUFFER_SIZE;
		scount += DAP_BUFFER_SECTORS;
		sector += DAP_BUFFER_SECTORS;
	}

	return 1;
}