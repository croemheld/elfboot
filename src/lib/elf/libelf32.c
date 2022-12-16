#include <elfboot/core.h>
#include <elfboot/libelf.h>
#include <elfboot/string.h>

/*
 * This file contains functions that can be applied to ELF32 objects.
 */

/*
 * Sections
 */

static Elf32_Shdr *__libelf32_shdr(Elf32_Ehdr *ehdr)
{
	return vptradd(ehdr, ehdr->e_shoff);
}

Elf32_Shdr *libelf32_shdr(Elf32_Ehdr *ehdr, Elf32_Half shndx)
{
	Elf32_Shdr *shdr = __libelf32_shdr(ehdr);

	return &shdr[shndx];
}

Elf32_Shdr *libelf32_find_shdr(Elf32_Ehdr *ehdr, const char *name)
{
	Elf32_Half shndx;

	for (shndx = 1; shndx < ehdr->e_shnum; shndx++) {
		if (strcmp(name, libelf32_section_name(ehdr, shndx)))
			continue;

		return libelf32_shdr(ehdr, shndx);
	}

	return NULL;
}

void *libelf32_section(Elf32_Ehdr *ehdr, Elf32_Half shndx)
{
	return vptradd(ehdr, libelf32_shdr(ehdr, shndx)->sh_offset);
}

void *libelf32_find_section(Elf32_Ehdr *ehdr, const char *name)
{
	Elf32_Half shndx;

	for (shndx = 1; shndx < ehdr->e_shnum; shndx++) {
		if (strcmp(name, libelf32_section_name(ehdr, shndx)))
			continue;

		return libelf32_section(ehdr, shndx);
	}

	return NULL;
}

char *libelf32_section_name(Elf32_Ehdr *ehdr, Elf32_Half shndx)
{
	void *strtable = libelf32_section(ehdr, ehdr->e_shstrndx);

	if (ehdr->e_shstrndx == SHN_UNDEF)
		return NULL;

	return vptradd(strtable, libelf32_shdr(ehdr, shndx)->sh_name);
}

/*
 * String table
 */

static char *libelf32_symbol_name(Elf32_Ehdr *ehdr, Elf32_Word sname)
{
	char *strtable = libelf32_find_section(ehdr, ELF_STRTAB);

	if (!strtable)
		return NULL;

	return strtable + sname;
}

/*
 * Symbol table
 */

static Elf32_Sym *libelf32_symbol(Elf32_Ehdr *ehdr, Elf32_Half syndx)
{
	Elf32_Sym *symtable = libelf32_find_section(ehdr, ELF_SYMTAB);

	if (!symtable)
		return NULL;

	return symtable + syndx;
}

static Elf32_Sym *__libelf32_find_symbol(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr,
	const char *name)
{
	Elf32_Word syndx;

	for (syndx = 0; syndx < (shdr->sh_size / shdr->sh_entsize); syndx++) {
		Elf32_Sym *sym = libelf32_symbol(ehdr, syndx);

		if (strcmp(name, libelf32_symbol_name(ehdr, sym->st_name)))
			continue;

		return sym;
	}

	return NULL;
}

Elf32_Sym *libelf32_find_symbol(Elf32_Ehdr *ehdr, const char *name)
{
	Elf32_Shdr *shdr = libelf32_find_shdr(ehdr, ELF_SYMTAB);

	if (!shdr)
		return NULL;

	return __libelf32_find_symbol(ehdr, shdr, name);
}