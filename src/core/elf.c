#include <elfboot/core.h>
#include <elfboot/elf.h>
#include <elfboot/string.h>

/*
 * Functions for 32-bit ELF files.
 */

bool is_elf_file(Elf32_Ehdr *ehdr)
{
	return memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0;
}

bool is_elf_sane(Elf32_Ehdr *ehdr)
{
	if (!is_elf_file(ehdr))
		return false;

	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
		return false;

	/*
	 * The following check has to be removed when compiling this bootloader
	 * for other architectures besides x86. Use types.h for byte conversion.
	 */

	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
		return false;

	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
		return false;

	if (ehdr->e_machine != EM_386)
		return false;

	if (ehdr->e_type != ET_REL && ehdr->e_type != ET_EXEC)
		return false;

	return true;
}

Elf32_Shdr *elf32_get_shdr(Elf32_Ehdr *ehdr)
{
	return vptradd(ehdr, ehdr->e_shoff);
}

Elf32_Shdr *elf32_get_shdr_shndx(Elf32_Ehdr *ehdr, Elf32_Half shndx)
{
	return elf32_get_shdr(ehdr) + shndx;
}

void *elf32_section_addr(Elf32_Ehdr *ehdr, Elf32_Half shndx)
{
	return vptradd(ehdr, elf32_get_shdr_shndx(ehdr, shndx)->sh_offset);
}

char *elf32_section_name(Elf32_Ehdr *ehdr, Elf32_Word shname)
{
	return elf32_section_addr(ehdr, ehdr->e_shstrndx) + shname;
}

void *elf32_find_section(Elf32_Ehdr *ehdr, const char *name, uint32_t *entnum)
{
	Elf32_Shdr *shdr;
	Elf32_Half shndx;

	for (shndx = 0; shndx < ehdr->e_shnum; shndx++) {
		shdr = elf32_get_shdr_shndx(ehdr, shndx);

		if (strcmp(name, elf32_section_name(ehdr, shdr->sh_name)))
			continue;

		if (entnum)
			*entnum = shdr->sh_size / shdr->sh_entsize;

		return elf32_section_addr(ehdr, shndx);
	}

	return NULL;
}