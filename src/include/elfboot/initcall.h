#ifndef __ELFBOOT_INITCALL_H__
#define __ELFBOOT_INITCALL_H__

#include <elfboot/core.h>
#include <elfboot/sections.h>

int elfboot_init(modinit_t *start, modinit_t *end);

#define elfboot_modinit(x)		\
		elfboot_init(__modinit_##x##_start, __modinit_##x##_end)

#define vfs_modinit()		elfboot_modinit(vfs)
#define dev_modinit()		elfboot_modinit(dev)

#endif /* __ELFBOOT_INITCALL_H__ */