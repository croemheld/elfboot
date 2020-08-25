#ifndef __ELFBOOT_LIBTMG_H__
#define __ELFBOOT_LIBTMG_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/file.h>
#include <elfboot/string.h>

#define TMG_SIGNATURE_LENGTH	3

struct tmg_header {
		const char sig[TMG_SIGNATURE_LENGTH];
		uint8_t version;
		uint8_t mode;
		uint8_t alpha;
		uint8_t width;
		uint8_t height;
		unsigned char image[0];
} __packed;

static inline bool libtmg_verify(struct tmg_header *tmg)
{
	return !strncmp(tmg->sig, "TMG", TMG_SIGNATURE_LENGTH);
}

struct tmg_header *libtmg_open(struct file *file);

#endif /* __ELFBOOT_LIBTMG_H__ */