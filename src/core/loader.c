#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/bdev.h>
#include <elfboot/file.h>
#include <elfboot/module.h>
#include <elfboot/loader.h>
#include <elfboot/console.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>
#include <elfboot/tree.h>
#include <elfboot/time.h>
#include <elfboot/printf.h>
#include <elfboot/string.h>
#include <elfboot/libtmg.h>

#include <uapi/elfboot/ioctls.h>

/*
 * List of recognized boot list entries from "/root/elfboot.cfg". The variable
 * boot_number stored the number of recognized boot entries in the list and is
 * also used to allocate the correct number of bytes for the boot option array
 */
LIST_HEAD(boot_list);
static int boot_number = 0;
static int boot_choice = 0;

/*
 * An array of the boot entries stored in the boot_list variable. This is done
 * in order to make navigating through the boot entries simpler.
 */
struct boot_entry *boot_options = NULL;

/*
 * Timeout in milliseconds after which an boot entry is selected and booted if
 * the user doesn't confirm the initial selection.
 */
static int boot_timeout = TIME_SECSTOMILLI(10);

/*
 * Console and attributes for interactive boot  menu. The menu attributes need
 * to be allocated in order to be used for redrawing during the interrupts.
 */
static struct file *menu = NULL;
static struct console_attr *menu_attr = NULL;

/*
 * Pointer to elfboot logo displayed above the boot menu entries. Currently we
 * only support TMG images for the logo.
 *
 * TODO CRO: Add support for other image formats in the future.
 */
struct tmg_header *logo;

/*
 * Messages to print on the boot menu
 */
const  char control[] = "Use arrow keys (\x18 and \x19) to change the selection";
const  char boption[] = "The selected entry will be chosen in %2d seconds.";
static char *boptstr = NULL;

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
	list_add_tail(&boot_entry->list, &boot_list);

	boot_number++;
	boot_choice++;
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

static int loader_init_boot_entries(struct file *file)
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

/*
 * Functions for graphical boot menu
 */

#ifdef CONFIG_DRIVER_TTY

static void loader_timeout_callback(void *info __unused)
{
	int secsold, secsnew;

	if (!boot_timeout)
		return;

	secsold = TIME_MILLITOSECS(boot_timeout--);
	secsnew = TIME_MILLITOSECS(boot_timeout);

	/*
	 * Check if at least one second has passed since the last console update.
	 * If yes, redraw the string which contains the number of seconds left.
	 */
	if (secsold == secsnew)
		return;

	boptstr = bmalloc(logo->width);
	if (!boptstr)
		return;

	menu_attr->xpos = (menu_attr->width - strlen(control)) / 2;
	menu_attr->ypos = menu_attr->height - 2;
	file_ioctl(menu, IOCTL_RESET, menu_attr);

	sprintf(boptstr, boption, secsnew);
	file_write(menu, logo->width, boptstr);

	bfree(boptstr);
}

static struct interrupt_handler loader_interrupt_handler = {
	.name = "Loader Timeout Handler",
	.callback = loader_timeout_callback
};

#endif /* CONFIG_DRIVER_TTY */

static int loader_boot_choice(void)
{
#ifdef CONFIG_DRIVER_TTY
	menu_attr->active = 1;
	menu_attr->xpos = 0;
	menu_attr->ypos = 0;
	file_ioctl(menu, IOCTL_CLEAN, menu_attr);
#endif /* CONFIG_DRIVER_TTY*/

	return loader_init_module(boot_options + boot_choice);
}

#ifdef CONFIG_DRIVER_TTY

static int loader_menu_font(int index, struct console_attr *cons_attr)
{
	if (index >= boot_number)
		return -EFAULT;

	if (index == boot_choice) {
		cons_attr->fgcolor = menu_attr->bgcolor;
		cons_attr->bgcolor = menu_attr->fgcolor;
	} else {
		cons_attr->fgcolor = menu_attr->fgcolor;
		cons_attr->bgcolor = menu_attr->bgcolor;
	}

	return 0;
}

static int loader_menu_show_entry(int index, struct console_attr *cons_attr)
{
	char *bent, *fent;
	struct boot_entry *entry = NULL;

	bent = bmalloc(logo->width);
	if (!bent)
		return -ENOMEM;

	fent = bmalloc(logo->width);
	if (!fent)
		goto loader_free_bent;

	cons_attr->ypos += index;

	if (loader_menu_font(index, cons_attr))
		goto loader_free_fent;

	file_ioctl(menu, IOCTL_RESET, cons_attr);

	/*
	 * We want to make each entry as long as the logo in order to make it
	 * look more beautiful :)
	 */
	entry = boot_options + index;
	sprintf(bent, " %s (%s) ", entry->name, entry->cmdline);
	sprintf(fent, "%-*s", logo->width, bent);

	file_write(menu, logo->width, fent);

loader_free_fent:
	bfree(fent);

loader_free_bent:
	bfree(bent);

	return entry == NULL;
}

static int loader_init_update(int prev_choice)
{
	struct console_attr cons_attr;

	menu_attr->ypos = 4 + (logo->height / 2);
	memcpy(&cons_attr, menu_attr, sizeof(*menu_attr));

	/*
	 * Update boot menu entry selection on the console. We have to get
	 * the boot entry and redraw both the old and the new selection.
	 */
	cons_attr.ypos = menu_attr->ypos + 3;
	if (loader_menu_show_entry(prev_choice, &cons_attr))
		return -EFAULT;

	cons_attr.ypos = menu_attr->ypos + 3;
	if (loader_menu_show_entry(boot_choice, &cons_attr))
		return -EFAULT;

	return 0;
}

static int loader_menu_init_selected(unsigned char c, int prev_choice)
{
	char *noption = bmalloc(logo->width);

	if (!noption)
		return -ENOMEM;

	if (!c || c == CONSOLE_CTRL_ENTER)
		return loader_boot_choice();

	/*
	 * Clear out message which contains number of seconds until auto boot.
	 * We do this after the user either selected an entry or if no key has
	 * been pressed after <selected_timeout> milliseconds.
	 */
	sprintf(noption, "%-*s", logo->width, "");
	file_ioctl(menu, IOCTL_RESET, menu_attr);
	file_write(menu, logo->width, noption);

	/*
	 * A key has been pressed but it wasn't CONSOLE_CTRL_ENTER. We have to
	 * wait until the user changes the current selection, and confirms the
	 * highlighted entry.
	 */
	while (c != CONSOLE_CTRL_ENTER) {

		if (c == CONSOLE_CTRL_ARRUP) {
			if (boot_choice <= 0)
				goto loader_menu_next_key;

			prev_choice = boot_choice--;
		} else if (c == CONSOLE_CTRL_ARRDO) {
			if (boot_choice >= boot_number - 1)
				goto loader_menu_next_key;

			prev_choice = boot_choice++;
		} else
			goto loader_menu_next_key;

		if (loader_init_update(prev_choice))
			return -EFAULT;

loader_menu_next_key:
		c = bgetch();
	}

	disable_keyboard();

	return loader_boot_choice();
}

static int loader_menu_init_keyboard(void)
{
	unsigned char c = 0;

	enable_keyboard();

	register_interrupt_handler(32, &loader_interrupt_handler);

	while (boot_timeout) {
		c = bgetch_noblock();
		if (c)
			break;
	}

	unregister_interrupt_handler(&loader_interrupt_handler);

	return loader_menu_init_selected(c, boot_choice);
}

static int loader_menu_show(struct console_attr *cons_attr)
{
	int index = 0;

	memcpy(cons_attr, menu_attr, sizeof(*cons_attr));

	cons_attr->ypos += 3;
	file_ioctl(menu, IOCTL_RESET, cons_attr);

	for (index = 0; index < boot_number; index++) {
		/*
		 * Draw each entry while simultaneously updating the console attributes
		 * via IOCTLs. For each index, the font has to be changed appropiately.
		 */
		if (loader_menu_show_entry(index, cons_attr))
			return -EFAULT;
	}

	return loader_menu_init_keyboard();
}

static int loader_menu_init(void)
{
	struct console_attr cons_attr;

	menu_attr->xpos = (menu_attr->width - logo->width) / 2;
	menu_attr->ypos = 2;

	file_ioctl(menu, IOCTL_RESET, menu_attr);
	file_ioctl(menu, IOCTL_PAINT, logo);

	/*
	 * Store the menu_attr values in the local variable since we don't want to
	 * modify the global values anymore since they are needed for redrawing.
	 */
	menu_attr->ypos = 4 + (logo->height / 2);
	memcpy(&cons_attr, menu_attr, sizeof(cons_attr));

	cons_attr.xpos = (menu_attr->width - strlen(control)) / 2;
	cons_attr.ypos = cons_attr.height - 3;
	file_ioctl(menu, IOCTL_RESET, &cons_attr);
	file_write(menu, logo->width, control);

	return loader_menu_show(&cons_attr);
}

#endif /* CONFIG_DRIVER_TTY */

static int loader_menu(void)
{
#ifdef CONFIG_DRIVER_TTY
	struct file *file;

	file = file_open("/root/elfboot.tmg", 0);
	if (!file)
		return -EFAULT;

	menu = file_open("/dev/tty", 0);
	if (!menu)
		goto loader_menu_free_file;

	logo = libtmg_open(file);
	if (!logo)
		goto loader_menu_free_menu;

	menu_attr = bmalloc(sizeof(*menu_attr));
	if (!menu_attr)
		goto loader_menu_free_logo;

	file_ioctl(menu, IOCTL_CLEAN, NULL);
	file_ioctl(menu, IOCTL_GATTR, menu_attr);

#ifdef CONFIG_LOADER_AUTOBOOT
	if (boot_number == 1)
		return loader_boot_choice();
#endif /* CONFIG_LOADER_AUTOBOOT */

	return loader_menu_init();

#else /* !CONFIG_DRIVER_TTY */

	return loader_boot_choice();

#endif /* CONFIG_DRIVER_TTY */

#ifdef CONFIG_DRIVER_TTY
loader_menu_free_logo:
	bfree(logo);

loader_menu_free_menu:
	bfree(menu);

loader_menu_free_file:
	bfree(file);

	return -EFAULT;
#endif /* CONFIG_DRIVER_TTY */
}

static int loader_init_boot_options(void)
{
	struct boot_entry *entry, *enptr;

	if (!boot_number)
		return -ENOENT;

	boot_options = bmalloc(sizeof(*entry) * boot_number);
	if (!boot_options)
		return -ENOMEM;

	list_for_each_entry(entry, &boot_list, list) {
		/*
		 * Copy entry in the dedicated position on the boot_options array to
		 * make navigation easier when redrawing the loader menu.
		 */
		enptr = boot_options + (boot_number - boot_choice--);
		memcpy(enptr, entry, sizeof(*entry));
	}

	return 0;
}

void loader_register(struct elf_loader *loader)
{
	list_add(&loader->list, &loaders);
}

int loader_init(void)
{
	struct file *bcfg;

	bcfg = file_open("/root/elfboot.cfg", 0);
	if (!bcfg)
		return -EFAULT;

	if (loader_init_boot_entries(bcfg))
		goto loader_free_bcfg;

	if (loader_init_boot_options())
		goto loader_free_bcfg;

	bfree(bcfg);

	return loader_menu();

loader_free_bcfg:
	bfree(bcfg);

	return -EFAULT;
}