#ifndef __ELFBOOT_MODULE_H__
#define __ELFBOOT_MODULE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/elf.h>
#include <elfboot/list.h>

#ifndef MODULE

#define module_init(init)	initcall_t initcall_##init __modinit = init
#define module_exit(exit)	exitcall_t exitcall_##exit __modexit = exit

#else

#define module_init(init)	int  init_module(void) __alias(#init)
#define module_exit(exit)	void exit_module(void) __alias(#exit)

#endif

struct module {
	char name[32];

	/* Module Elf object */
	union {
		void *buffer;
		Elf32_Ehdr *ehdr;
	};
	Elf32_Shdr *shdr;
	Elf32_Sym *symtab;
	uint32_t numsyms;
	char *strtab;
	char *shstrtab;

	/* Init & exit */
	int (*init)(void);
	void (*exit)(void);

	/* List of modules */
	struct list_head list;
};

int module_open(const char *path);

void modules_exit(void);

int modules_init(void);

int parse_module_map(char *table);

#endif /* __ELFBOOT_MODULE_H__ */