#ifndef __BOOT_ELF32_H__
#define __BOOT_ELF32_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <boot/elf.h>

struct elf32_hdr {
	struct elf_hdr hdr;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} __attribute__((packed));

struct elf32_shdr {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} __attribute__((packed));

struct elf32_sym {
	uint32_t st_name;
	uint32_t st_value;
	uint32_t st_size;
	unsigned char st_info;
	unsigned char st_other;
	uint16_t st_shndx;
} __attribute__((packed));

struct elf32_rel {
	uint32_t r_offset;
	uint32_t r_info;
} __attribute__((packed));

struct elf32_rela {
	uint32_t r_offset;
	uint32_t r_info;
	int32_t  r_addend;
} __attribute__((packed));

struct elf32_phdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} __attribute__((packed));

struct elf32_dyn {
	int32_t  d_tag;
	union {
		uint32_t d_val;
		uint32_t d_ptr;
	} d_un;
} __attribute__((packed));

bool elf32_check_file(struct elf32_hdr *elf32_hdr);

struct elf32_shdr *elf32_get_shdr(struct elf32_hdr *elf32_hdr);

struct elf32_shdr *elf32_get_shdr_by_index(struct elf32_hdr *elf32_hdr, int index);

char *elf32_get_shdr_strtab(struct elf32_hdr *elf32_hdr);

char *elf32_get_shdr_name(struct elf32_hdr *elf32_hdr, struct elf32_shdr *elf32_shdr);

struct elf32_shdr *elf32_get_shdr_by_name(struct elf32_hdr *elf32_hdr, const char *name);

struct elf32_phdr *elf32_get_phdr(struct elf32_hdr *elf32_hdr);

#endif /* __BOOT_ELF32_H__ */