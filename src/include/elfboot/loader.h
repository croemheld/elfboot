#ifndef __ELFBOOT_LOADER_H__
#define __ELFBOOT_LOADER_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

#define LINE_DELIMITER				"\n"
#define NAME_DELIMITER				"\""
#define INFO_DELIMITER				"\t"
#define WORD_DELIMITER				" "
#define BOOT_DELIMITER				"}"

#define CHAR_COMMENT				"#"

#define C(x)						x[0]

#define LOADER_PROTOCOL_MULTIBOOT1	1
#define LOADER_PROTOCOL_MULTIBOOT2	2
#define LOADER_PROTOCOL_LINUX		3

struct boot_entry {
	char *name;
	char *kernel_path;
	char *initrd_path;
	char *cmdline;
	uint32_t prot;
	struct list_head list;
};

struct elf_loader {
	int prot;
	int (*boot)(struct boot_entry *);
	struct list_head list;
};

void loader_register(struct elf_loader *loader);

int loader_init(void);

#endif /* __ELFBOOT_LOADER_H__ */