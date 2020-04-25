#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/string.h>
#include <elfboot/sections.h>

void bfree_const(const void *ptr)
{
	uint32_t addr = tuint(ptr);

	if (!is_elfboot_rodata(addr))
		bfree((void *)ptr);
}

char *bstrdup(const char *str)
{
	size_t len;
	char *buf;

	if (!str)
		return NULL;

	len = strlen(str) + 1;
	buf = bmalloc(len);

	if (!buf)
		return NULL;

	memcpy(buf, str, len);

	return buf;
}