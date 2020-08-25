#ifndef __ELFBOOT_MODULE_H__
#define __ELFBOOT_MODULE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/elf.h>
#include <elfboot/list.h>

#ifndef MODULE

/*
 * For built-in modules, we have to make sure to place the init calls in order,
 * otherwise the initialization process may fail.
 */
#define module_init(init)		modinit_t modinit_##init __modinit = init

#define vfs_module_init(init)	modinit_t modinit_##init __modinit_vfs = init
#define dev_module_init(init)	modinit_t modinit_##init __modinit_dev = init

/*
 * Exitcalls are unordered, we don' have to call them in order.
 */
#define module_exit(exit)		modexit_t modexit_##exit __modexit = exit

#else /* !MODULE */

/*
 * If we initialize an external module, there is only one module initialization
 * function for all kind of modules.
 */
#define module_init(init)		int  init_module(void) __alias(#init)

#define vfs_module_init(init)	module_init(init)
#define dev_module_init(init)	module_init(init)

#define module_exit(exit)		void exit_module(void) __alias(#exit)

#endif /* MODULE */

#define vfs_module_exit(exit)	module_exit(exit)
#define dev_module_exit(exit)	module_exit(exit)

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

int module_open(const char *name);

void modules_exit(void);

int modules_init(void);

#endif /* __ELFBOOT_MODULE_H__ */