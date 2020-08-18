#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/bdev.h>
#include <elfboot/file.h>
#include <elfboot/module.h>
#include <elfboot/loader.h>
#include <elfboot/tree.h>
#include <elfboot/printf.h>
#include <elfboot/string.h>

/*
 * List of recognized boot entries from /root/elfboot.cfg.
 */
LIST_HEAD(boot_entries);
static int num_boot_entries = 0;

/*
 * Helper array for loading the appropiate loader module.
 */
static const char *loader_modules[] = {
	[LOADER_PROTOCOL_MULTIBOOT] = "mbboot",
	[LOADER_PROTOCOL_LINUX] 	= "lxboot"
};

/*
 * List of registered ELF loaders.
 */
LIST_HEAD(loaders);

/*
 * Functions for parsing boot entries
 */

static char *loader_parse_name(char *line)
{
	char *nbeg, *nend, *name = NULL;

	if ((nbeg = strstr(line, NAME_DELIMITER))) {
		nbeg++;

		if ((nend = strstr(nbeg, NAME_DELIMITER))) {
			name = bmalloc(nend - nbeg + 1);
			if (!name)
				return NULL;

			memcpy(name, nbeg, nend - nbeg);
			name[nend - nbeg] = 0;
		}
	}

	return name;
}

static char *loader_strip_entry_line(char *line)
{
	while (!strncmp(line, WORD_DELIMITER, 1) || !strncmp(line, INFO_DELIMITER, 1))
		line++;

	return line;
}

static struct boot_entry *loader_create_entry(char *name)
{
	struct boot_entry *boot_entry = bzalloc(sizeof(*boot_entry));

	if (!boot_entry)
		return NULL;

	boot_entry->name = name;

	return boot_entry;
}

static void loader_free_entry(struct boot_entry *boot_entry)
{
	if (boot_entry->name)
		bfree(boot_entry->name);

	list_del(&boot_entry->list);

	bfree(boot_entry);
}

static int loader_parse_kernel(struct boot_entry *boot_entry, char *option)
{
	char *lopt, *topt = strtok_r(option, WORD_DELIMITER, &lopt);

	while (topt) {
		if (!strcmp(topt, "multiboot")) {
			boot_entry->prot = LOADER_PROTOCOL_MULTIBOOT;
			goto parse_next_kernel_option;
		} else if (!strcmp(topt, "linux")) {
			boot_entry->prot = LOADER_PROTOCOL_LINUX;
			goto parse_next_kernel_option;
		} else {
			if (!boot_entry->prot)
				return -EFAULT;

			boot_entry->kernel_path = bstrdup(topt);
			if (!boot_entry->kernel_path)
				return -ENOMEM;

			break;
		}

parse_next_kernel_option:
		topt = strtok_r(NULL, WORD_DELIMITER, &lopt);
	}

	return 0;
}

static int loader_parse_initrd(struct boot_entry *boot_entry, char *option)
{
	char *lopt, *topt = strtok_r(option, WORD_DELIMITER, &lopt);

	while (topt) {
		boot_entry->initrd_path = bstrdup(topt);
		if (!boot_entry->initrd_path)
			return -ENOMEM;

		topt = strtok_r(NULL, WORD_DELIMITER, &lopt);
	}

	return 0;
}

static int loader_parse_cmdline(struct boot_entry *boot_entry, char *option)
{
	boot_entry->cmdline = bstrdup(option);
	if (!boot_entry->cmdline)
		return -ENOMEM;

	return 0;
}

static int loader_parse_entry(struct boot_entry *boot_entry, char *line)
{
	char *option = loader_strip_entry_line(line);

	if (!strncmp(option, "kernel", 6))
		return loader_parse_kernel(boot_entry, option + 7);
	if (!strncmp(option, "initrd", 6))
		return loader_parse_initrd(boot_entry, option + 7);
	if (!strncmp(option, "cmdline", 7))
		return loader_parse_cmdline(boot_entry, option + 8);

	bprintln("EBL: Unsupported bootentry option \"%s\"", option);

	return 0;
}

static void loader_store_entry(struct boot_entry *boot_entry)
{
	list_add(&boot_entry->list, &boot_entries);

	num_boot_entries++;
}

static int loader_parse_entries(char *config)
{
	char *lend, *line, *name;
	struct boot_entry *boot_entry = NULL;

	line = strtok_r(config, LINE_DELIMITER, &lend);

	while (line) {
		if (*line == C(CHAR_COMMENT))
			continue;

		if (!strncmp(line, "bootentry", 9)) {
			if (boot_entry)
				loader_free_entry(boot_entry);

			name = loader_parse_name(line);
			if (!name)
				goto parse_next_line;

			boot_entry = loader_create_entry(name);
			if (!boot_entry) {
				bfree(name);
				goto parse_next_line;
			}
		} else if (*line == C(INFO_DELIMITER)) {
			if (!boot_entry)
				goto parse_next_line;

			if (loader_parse_entry(boot_entry, line))
				goto parse_next_line;
		} else if (*line == C(BOOT_DELIMITER)) {
			if (!boot_entry)
				goto parse_next_line;

			loader_store_entry(boot_entry);
			boot_entry = NULL;
		}

parse_next_line:
		line = strtok_r(NULL, LINE_DELIMITER, &lend);
	}

	return 0;
}

static int loader_parse_config(struct file *file)
{
	char *config = bmalloc(file->length);

	if (!config)
		return -ENOMEM;

	if (!file_read(file, file->length, config))
		goto loader_free_config;

	if (loader_parse_entries(config))
		goto loader_free_config;

	return 0;

loader_free_config:
	bfree(config);

	return -ENOMEM;
}

/*
 * Functions for booting kernel
 */

static struct elf_loader *loader_get_loader(int prot)
{
	struct elf_loader *loader;

	list_for_each_entry(loader, &loaders, list) {
		if (loader->prot == prot)
			return loader;
	}

	return NULL;
}

static int loader_boot_kernel(struct boot_entry *boot_entry)
{
	struct elf_loader *loader = loader_get_loader(boot_entry->prot);

	if (!loader)
		return -EFAULT;

	return loader->boot(boot_entry);
}

static int loader_init_module(struct boot_entry *boot_entry)
{
	const char *name;

	if (boot_entry->prot >= ARRAY_SIZE(loader_modules))
		return -ENOTSUP;

	name = loader_modules[boot_entry->prot];

	bprintln("EBL: Loading module %s", name);

	if (module_open(name))
		return -EFAULT;

	return loader_boot_kernel(boot_entry);
}

static int loader_list_entries(void)
{
	struct boot_entry *boot_entry;

	if (!num_boot_entries)
		return -ENOENT;

	bprintln("EBL: Found %ld boot entries", num_boot_entries);

	if (num_boot_entries == 1) {
		boot_entry = list_entry(boot_entries.next, struct boot_entry, list);
		loader_init_module(boot_entry);
	} else {
		/*
		 * We found more than one boot entry. Give the user the opportunity to
		 * select a boot entry via keyboard. For this feature, interrupts have
		 * to be enabled and the IDT has to be set up.
		 */

		list_for_each_entry(boot_entry, &boot_entries, list)
			bprintln("EBL: Found entry \"%s\"", boot_entry->name);

		return -ENOTSUP;
	}

	return 0;
}

void loader_register(struct elf_loader *loader)
{
	list_add(&loader->list, &loaders);
}

int loader_init(void)
{
	struct file *file = file_open("/root/elfboot.cfg", 0);

	if (!file)
		return -EFAULT;

	num_boot_entries = 0;

	if (loader_parse_config(file))
		goto loader_free_file;

	bfree(file);

	return loader_list_entries();

loader_free_file:
	bfree(file);

	return -EFAULT;
}