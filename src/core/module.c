#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/elf.h>
#include <elfboot/file.h>
#include <elfboot/module.h>
#include <elfboot/symbol.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

LIST_HEAD(modules);

static int module_find_sections(struct module *mod)
{
	mod->shdr = elf32_get_shdr(mod->ehdr);

	/* 
	 * Assign .shstrtab section first to search for sections by name. This is
	 * cleaner, even though it requires iterating the section headers several
	 * times. Since this is a bootloader, we don't care about performance too
	 * much...
	 */
	mod->shstrtab = elf32_section_addr(mod->ehdr, mod->ehdr->e_shstrndx);

	/* 
	 * Search for ".symtab" and ".strtab" section by nyme by using the macros
	 * ELF_SYMTAB and ELF_STRTAB. This needs to happen as soon as the section
	 * ".shstrtab" has been assigned.
	 */
	mod->symtab = elf32_find_section(mod->ehdr, ELF_SYMTAB, &mod->numsyms);
	if (!mod->symtab)
		return -ENOENT;

	mod->strtab = elf32_find_section(mod->ehdr, ELF_STRTAB, NULL);
	if (!mod->strtab)
		return -ENOENT;

	return 0;
}

static int module_load_sections(struct module *mod)
{
	Elf32_Shdr *shdr;
	Elf32_Half shndx;

	for (shndx = 0; shndx < mod->ehdr->e_shnum; shndx++) {
		shdr = elf32_get_shdr_shndx(mod->ehdr, shndx);

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
	uint32_t symndx, bsymval;
	Elf32_Sym *symtab;

	symtab = mod->symtab;

	for (symndx = 1; symndx < mod->numsyms; symndx++) {
		const char *name = mod->strtab + symtab[symndx].st_name;

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
		shdr = elf32_get_shdr_shndx(mod->ehdr, shndx);

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
	uint32_t symndx;
	Elf32_Sym *sym;

	for (symndx = 0; symndx < mod->numsyms; symndx++) {
		sym = &mod->symtab[symndx];

		if (!strncmp(mod->strtab + sym->st_name, "init_module", 11))
			mod->init = tvptr(sym->st_value);

		if (!strncmp(mod->strtab + sym->st_name, "exit_module", 11))
			mod->exit = tvptr(sym->st_value);
	}

	if (!mod->init || !mod->exit)
		return -ENOEXEC;

	if (mod->init())
		return -EFAULT;

	mod->name = bstrdup(mod->file->name);
	if (!mod->name)
		return -ENOMEM;

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

	mod->file = file;

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

	return 0;

module_free_buffer:
	bfree(mod->buffer);

module_free_mod:
	bfree(mod);

	return -EFAULT;
}

int module_open(const char *path)
{
	struct file *file = file_open(path, 0);

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
	exitcall_t exitcall, *function = __exitcalls_start;

	for (; function < __exitcalls_end; function++) {
		exitcall = *function;
		exitcall();
	}
}

int modules_init(void)
{
	initcall_t initcall, *function = __initcalls_start;

	for (; function < __initcalls_end; function++) {
		initcall = *function;

		/*
		 * Iterate over all initcalls in the dedicated initcall section of the
		 * elfboot bootloader. If even one initcall fails, we call module_exit
		 * to make sure we cleared up everything.
		 */
		if (initcall())
			modules_exit();
	}

	return 0;
}