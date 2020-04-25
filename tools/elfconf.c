/*
 * elfconf - ELF manipulation tool
 *
 * This file is a compact, 32-bit ELF only file for modifying the stages
 * of the elfboot bootloader. The original elfconf project can be found
 * in the URL below:
 *
 * https://github.com/croemheld/elfconf
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <elf.h>

/*
 * Special macros
 */

#define ELFCONF_SECTION_SYMTAB  ".symtab"
#define ELFCONF_SECTION_STRTAB  ".strtab"

/*
 * Structures and typedefs
 */

struct elfconf_arguments {
	/*
	 * Arguments from command line
	 */
	char *elf;
	char *sym;
	unsigned long val;
	/*
	 * FILE pointer and ELF buffer
	 */
	FILE *efp;
	void *buf;
};

struct elfconf_elf32file {
	union {
		void *head;
		Elf32_Ehdr *ehdr;
	};
	Elf32_Shdr *shdr;
	Elf32_Sym *symtab;
	unsigned int numsyms;
	char *strtab;
	char *shstrtab;
};

/*
 * General ELF helper functions and macros (for both 32-bit and 64-bit)
 */

/* Section relevant macros */
#define elf_section_header(elf, shndx)	(elf)->shdr + shndx
#define elf_section(elf, shndx)		(elf)->head + ((elf)->shdr + shndx)->sh_offset
#define elf_section_name(elf, name)	(elf)->shstrtab + name

/* Symbol relevant macros */
#define elf_symbol(elf, symndx)		(elf)->symtab + symndx
#define elf_symbol_name(elf, sym)	(elf)->strtab + (sym)->st_name

/* Get the offset of a symbol in the ELF binary */
#define elf_symbol_offset(elf, sym)	 ({		\
	typeof((elf)->shdr) __section = elf_section_header(elf, (sym)->st_shndx);	\
	(sym)->st_value - (__section)->sh_addr + (__section)->sh_offset;			\
})

#define elf_symbol_bind(sym)    (((sym)->st_info) >>  4)
#define elf_symbol_type(sym)    (((sym)->st_info) & 0xf)

/*
 * Parsing ELF file
 */

static inline void *elf_offset(struct elfconf_arguments *args, unsigned long offset) {
	return args->buf + offset;
}

static void clear_elfconf_file(struct elfconf_arguments *args) {
	if (args->efp)
		fclose(args->efp);

	if (args->buf)
		free(args->buf);
}

/*
 * 32-bit ELF functions
 */

static void *find_elf32_section(struct elfconf_elf32file *elf, char *name, unsigned int *num) {
	Elf32_Shdr *section;
	unsigned int shndx;

	for (shndx = 0; shndx < elf->ehdr->e_shnum; shndx++) {
		section = elf_section_header(elf, shndx);

		if (strcmp(name, elf_section_name(elf, section->sh_name)))
			continue;

		if (num)
			*num = section->sh_size / section->sh_entsize;

		return elf_section(elf, shndx);
	}

	return NULL;
}

static int parse_elf32_file(struct elfconf_arguments *args, struct elfconf_elf32file *elf) {
	/* Initialize pointers to section headers */
	elf->ehdr = elf_offset(args, 0);
	elf->shdr = elf_offset(args, elf->ehdr->e_shoff);

	/* Assign .shstrtab section first */
	elf->shstrtab = elf_section(elf, elf->ehdr->e_shstrndx);

	/* Search for .symtab and .strtab section */
	elf->symtab = find_elf32_section(elf, ELFCONF_SECTION_SYMTAB, &elf->numsyms);
	if (!elf->symtab)
		return -ENAVAIL;

	elf->strtab = find_elf32_section(elf, ELFCONF_SECTION_STRTAB, NULL);
	if (!elf->strtab)
		return -ENAVAIL;

	return 0;
}

static Elf32_Sym *find_elf32_symbol(struct elfconf_elf32file *elf, char *name) {
	Elf32_Sym *symbol;
	unsigned int index;

	for(index = 0; index < elf->numsyms; index++) {
		symbol = elf_symbol(elf, index);
		if (strcmp(name, elf_symbol_name(elf, symbol)))
			continue;

		if (symbol->st_shndx == SHN_UNDEF)
			return NULL;

		return symbol;
	}

	return NULL;
}

static int configure_elf32_symbol(struct elfconf_arguments *args, struct elfconf_elf32file *elf) {
	Elf32_Sym *symbol;
	size_t offset, write;

	symbol = find_elf32_symbol(elf, args->sym);
	if (!symbol)
		return -ENAVAIL;

	/* Move pointer to the symbol in the ELF file */
	offset = elf_symbol_offset(elf, symbol);
	fseek(args->efp, offset, SEEK_SET);

	/* Write new value at the specified symbol */
	write = fwrite(&args->val, 1, symbol->st_size, args->efp);
	if (write != symbol->st_size)
		return -EBADFD;

	return 0;
}

static int apply_elf32_args(struct elfconf_arguments *args) {
	struct elfconf_elf32file elf;

	/* Fill up data structure */
	if (parse_elf32_file(args, &elf))
		return -EFAULT;

	/* Search and modify symbol */
	if (configure_elf32_symbol(args, &elf))
		return -EFAULT;

	return 0;
}

static int parse_elfconf_file(struct elfconf_arguments *args) {
	Elf32_Ehdr *ehdr = args->buf;

	/* Validate ELF signature at beginning of file */
	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG))
		return -ENOTSUP;

	/*
	 * Determine ELF class: 32-bit or 64-bit
	 */

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
		return apply_elf32_args(args);

	return -ENOTSUP;
}

static int apply_elfconf_args(struct elfconf_arguments *args) {
	size_t size, read;

	/* Open ELF file */
	args->efp = fopen(args->elf, "rb+");
	if (!args->efp)
		return -EBADFD;

	/* Get ELF size */
	fseek(args->efp, 0L, SEEK_END);
	size = ftell(args->efp);

	/* Alloc buffer and read ELF */
	args->buf = malloc(size);
	if (!args->buf) {
		clear_elfconf_file(args);
		return -ENOMEM;
	}

	fseek(args->efp, 0L, SEEK_SET);
	read = fread(args->buf, 1, size, args->efp);
	if (read != size) {
		clear_elfconf_file(args);
		return -EBADFD;
	}

	if (parse_elfconf_file(args)) {
		clear_elfconf_file(args);
		return -EFAULT;
	}

	return 0;
}

/*
 * Parsing arguments
 */

static int parse_elfconf_args(int argc, char *argv[], struct elfconf_arguments *args)
{
	int option;

	/*
	 * Options supported by elfconf:
	 *
	 * -h: Show usage of this program and quit.
	 *
	 * The following options need to be specified together:
	 *
	 * -f: ELF input file to be manipulated.
	 * -s: Symbol name in ELF which we want to modify.
	 * -v: The value that should be written to the symbol.
	 */

	while ((option = getopt(argc, argv, "f:s:v:")) != -1) {
		switch (option) {
			case 'f':
				args->elf = optarg;
				break;
			case 's':
				args->sym = optarg;
				break;
			case 'v':
				args->val = strtoull(optarg, NULL, 0);
				break;
			case '?':
				return -EFAULT;
			default:
				abort();
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
	struct elfconf_arguments args;

	if (parse_elfconf_args(argc, argv, &args))
		return -EFAULT;

	if (apply_elfconf_args(&args))
		return -EFAULT;

	clear_elfconf_file(&args);

	return 0;
}
