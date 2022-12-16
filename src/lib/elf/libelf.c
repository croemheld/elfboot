#include <elfboot/core.h>
#include <elfboot/libelf.h>
#include <elfboot/string.h>

/*
 * This file contains functions that can be applied to both ELF32 and ELF64
 * objects. For this reason, some return values or function arguments use a
 * void pointer that points to either object types.
 */

bool libelf_compare_magic(void *ehdr)
{
	return memcmp(ehdr, ELFMAG, SELFMAG) == 0;
}

bool libelf_compare_ident(void *ehdr, unsigned char ident, unsigned char value)
{
	unsigned char *field = ehdr + ident;

	return *(field) == value;
}

bool libelf_compare_class(void *ehdr, unsigned char class)
{
	return libelf_compare_ident(ehdr, EI_CLASS, class);
}

bool libelf_confirm_ident(void *ehdr, unsigned char class)
{
	if (!libelf_compare_magic(ehdr))
		return false;

	if (!libelf_compare_class(ehdr, class))
		return false;

	return true;
}

struct elf_handle {
	struct file *file;
	union {
		void *ehdr;
		Elf32_Ehdr *ehdr32;
		Elf64_Ehdr *ehdr64;
	};
	union {
		void *shdr;
		Elf32_Shdr *shdr32;
		Elf64_Shdr *shdr64;
	};
	union {
		void *symtab;
		Elf32_Sym *symtab32;
		Elf64_Sym *symtab64;
	};
	uint32_t numsyms;
	char *shstrtab;
	char *strtab;
};

/*
struct elf_handle *libelf_handle(struct file *file)
{
	unsigned char *ident = bmalloc(EI_NIDENT);

	if (!ident)
		return NULL;

	file_seek(file, FILE_SET, 0);
	if (!file_read(file, EI_NIDENT, ident))
		goto libelf_handle_fail;

	if (!libelf_compare_ident(ident, EI_CLASS, ELFCLASS32)) {
		bfree(ident);
		return libelf32_handle(file);
	}

	if (!libelf_compare_ident(ident, EI_CLASS, ELFCLASS64)) {
		bfree(ident);
		return libelf64_handle(file);
	}

libelf_handle_fail:
	bfree(ident);

	return NULL;
}
*/