#ifndef __ELFBOOT_INITCALL_H__
#define __ELFBOOT_INITCALL_H__

#include <elfboot/core.h>
#include <elfboot/sections.h>

int elfboot_init(initcall_t *start, initcall_t *end);

#define elfboot_initcall(x)		\
		elfboot_init(__initcalls_##x##_start, __initcalls_##x##_end)

#define vfs_initcalls()		elfboot_initcall(vfs)
#define dev_initcalls()		elfboot_initcall(dev)

#endif /* __ELFBOOT_INITCALL_H__ */