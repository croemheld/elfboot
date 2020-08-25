#include <elfboot/core.h>
#include <elfboot/file.h>
#include <elfboot/libtmg.h>

struct tmg_header *libtmg_open(struct file *file)
{
	struct tmg_header *tmg = bmalloc(file->length);

	if (!tmg)
		return NULL;

	file_read(file, file->length, tmg);

	if (!libtmg_verify(tmg))
		goto libtmg_free_file;

	return tmg;

libtmg_free_file:
	bfree(tmg);

	return NULL;
}