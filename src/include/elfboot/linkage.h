#ifndef __ELFBOOT_LINKAGE_H__
#define __ELFBOOT_LINKAGE_H__

#include <elfboot/core.h>

#include <asm/linkage.h>

#define __section(x)			__attribute__((section(x)))


#define __unused			__attribute__((unused))
#define __packed			__attribute__((packed))

#define __aligned(x)			__attribute__((aligned(x)))

#endif /* __ELFBOOT_LINKAGE_H__ */