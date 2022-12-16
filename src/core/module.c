#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/file.h>
#include <elfboot/module.h>
#include <elfboot/symbol.h>
#include <elfboot/sections.h>
#include <elfboot/libelf.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

LIST_HEAD(modules);

static Elf32_Sym *module_find_symbol(struct module *mod, const char *name)
{
	uint32_t symndx;
	Elf32_Sym *sym;

	for (symndx = 0; symndx < mod->numsyms; symndx++) {
		sym = &mod->symtab[symndx];

		if (strcmp(name, mod->strtab + sym->st_name))
			continue;

		return sym;
	}

	return NULL;
}

static int module_find_sections(struct module *mod)
{
	Elf32_Shdr *shdr;

	mod->shdr = libelf32_shdr(mod->ehdr, 0);

	/* 
	 * Assign .shstrtab section first to search for sections by name. This is
	 * cleaner, even though it requires iterating the section headers several
	 * times. Since this is a bootloader, we don't care about performance too
	 * much...
	 */
	mod->shstrtab = libelf32_section(mod->ehdr, mod->ehdr->e_shstrndx);

	/* 
	 * Search for ".symtab" and ".strtab" section by name by using the macros
	 * ELF_SYMTAB and ELF_STRTAB. This needs to happen as soon as the section
	 * ".shstrtab" has been assigned.
	 */
	shdr = libelf32_find_shdr(mod->ehdr, ELF_SYMTAB);
	if (!shdr)
		return -ENOENT;

	mod->symtab = libelf32_find_section(mod->ehdr, ELF_SYMTAB);
	mod->strtab = libelf32_find_section(mod->ehdr, ELF_STRTAB);
	if (!mod->strtab)
		return -ENOENT;

	mod->numsyms = shdr->sh_size / shdr->sh_entsize;

	return 0;
}

static int module_load_sections(struct module *mod)
{
	Elf32_Shdr *shdr;
	Elf32_Half shndx;

	for (shndx = 0; shndx < mod->ehdr->e_shnum; shndx++) {
		shdr = libelf32_shdr(mod->ehdr, shndx);

		if (shdr->sh_type == SHT_NOBITS) {
			shdr->sh_addr = tuint(bmalloc(shdr->sh_size));
			if (!shdr->sh_addr)
				return -ENOMEM;

			memset(tvptr(shdr->sh_addr), 0, shdr->sh_size);
		} else
			shdr->sh_addr = tuint(mod->ehdr) + shdr->sh_offset;
	}

	return 0;
}

static int module_resolve_symbols(struct module *mod)
{
	Elf32_Half symndx;
	Elf32_Sym *symtab;
	uint32_t bsymval;
	char *name;

	symtab = mod->symtab;

	for (symndx = 1; symndx < mod->numsyms; symndx++) {
		name = mod->strtab + symtab[symndx].st_name;

		switch (symtab[symndx].st_shndx) {
		case SHN_COMMON:
		case SHN_ABS:
		case SHN_LIVEPATCH:
			break;
		case SHN_UNDEF:
			bsymval = symbol_lookup_name(name);
			if (bsymval) {
				symtab[symndx].st_value = bsymval;
				break;
			} else {
				bprintln("MOD: Error: Symbol %s not found", name);
				return -ENOENT;
			}

			if (!bsymval && ELF_ST_BIND(symtab[symndx].st_info == STB_WEAK))
				break;

			break;
		default:
			bsymval = mod->shdr[symtab[symndx].st_shndx].sh_addr;
			symtab[symndx].st_value += bsymval;
			break;
		}
	}

	return 0;
}

static int module_apply_relocate(struct module *mod, Elf32_Shdr *shdr)
{
	Elf32_Rel *rel;
	Elf32_Sym *sym;
	Elf32_Word ndx;
	uint32_t *pos;

	rel = tvptr(shdr->sh_addr);

	for (ndx = 0; ndx < shdr->sh_size / sizeof(*rel); ndx++) {
		pos = tvptr(mod->shdr[shdr->sh_info].sh_addr + rel[ndx].r_offset);
		sym = mod->symtab + ELF32_R_SYM(rel[ndx].r_info);

		switch (ELF32_R_TYPE(rel[ndx].r_info)) {
		case R_386_32:
			*pos += sym->st_value;
			break;
		case R_386_PC32:
			*pos += sym->st_value - tuint(pos);
			break;
		default:
			return -ENOEXEC;
		}
	}

	return 0;
}

static int module_resolve_relocations(struct module *mod)
{
	Elf32_Shdr *shdr;
	Elf32_Half shndx;
	int ret;

	for (shndx = 1; shndx < mod->ehdr->e_shnum; shndx++) {
		shdr = libelf32_shdr(mod->ehdr, shndx);

		if (shdr->sh_info >= mod->ehdr->e_shnum)
			continue;

		if (shdr->sh_flags & SHF_RELA_LIVEPATCH)
			continue;

		if (shdr->sh_type == SHT_REL)
			ret = module_apply_relocate(mod, shdr);

		if (ret)
			break;
	}

	return 0;
}

static int module_initialize(struct module *mod)
{
	Elf32_Sym *init_sym, *exit_sym;

	init_sym = module_find_symbol(mod, "init_module");
	exit_sym = module_find_symbol(mod, "exit_module");

	if (init_sym)
		mod->init = tvptr(init_sym->st_value);

	if (exit_sym)
		mod->exit = tvptr(exit_sym->st_value);

	if (!mod->init || !mod->exit)
		return -ENOEXEC;

	if (mod->init())
		return -EFAULT;

	list_add(&mod->list, &modules);

	return 0;
}

static int module_read_file(struct file *file)
{
	struct module *mod = bmalloc(sizeof(*mod));

	if (!mod)
		return -ENOMEM;

	mod->buffer = bmalloc(file->length);
	if (!mod->buffer)
		goto module_free_mod;

	if (!file_read(file, file->length, mod->buffer))
		goto module_free_buffer;

	if (module_find_sections(mod))
		goto module_free_buffer;

	if (module_load_sections(mod))
		goto module_free_buffer;

	if (module_resolve_symbols(mod))
		goto module_free_buffer;

	if (module_resolve_relocations(mod))
		goto module_free_buffer;

	if (module_initialize(mod))
		goto module_free_buffer;

	strncpy(mod->name, file->name, strlen(file->name) - 4);

	return 0;

module_free_buffer:
	bfree(mod->buffer);

module_free_mod:
	bfree(mod);

	return -EFAULT;
}

int module_open(const char *name)
{
	struct file *file;
	char path[64] = { 0 };

	sprintf(path, "/root/modules/%s.ebm", name);

	file = file_open(path, 0);
	if (!file)
		return -ENOENT;

	if (module_read_file(file))
		goto module_free_file;

	return 0;

module_free_file:
	bfree(file);

	return -EFAULT;
}

void modules_exit(void)
{
	modexit_t modexit, *function = __modexit_start;

	for (; function < __modexit_end; function++) {
		modexit = *function;
		modexit();
	}
}

int modules_init(void)
{
	modinit_t modinit, *function = __modinit_start;

	for (; function < __modinit_end; function++) {
		modinit = *function;

		/*
		 * Iterate over all modinits in the dedicated modinit section of the
		 * elfboot bootloader. If even one modinit fails, we call module_exit
		 * to make sure we cleared up everything.
		 */
		if (modinit())
			return -EFAULT;
	}

	return 0;
}