#ifndef __BOOT_ELF_H__
#define __BOOT_ELF_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define EI_NIDENT                                 16
#define EI_MAG0                                    0
#define EI_MAG1                                    1
#define EI_MAG2                                    2
#define EI_MAG3                                    3
#define EI_CLASS                                   4
#define EI_DATA                                    5
#define EI_VERSION                                 6
#define EI_OSABI                                   7
#define EI_ABIVERSION                              8
#define EI_PAD                                     9

#define ELFMAG0                                   0x7f
#define ELFMAG1                                   'E'
#define ELFMAG2                                   'L'
#define ELFMAG3                                   'F'

/*
 * Object file classes
 */

#define ELFCLASSNONE                               0
#define ELFCLASS32                                 1
#define ELFCLASS64                                 2

/*
 * Encodings
 */

#define ELFDATANONE                                0
#define ELFDATA2LSB                                1
#define ELFDATA2MSB                                2

/*
 * Special section indexes
 */

#define SHN_UNDEF                                  0
#define SHN_LORESERVE                             0xff00
#define SHN_LOPROC                                0xff00
#define SHN_HIPROC                                0xff1f
#define SHN_ABS                                   0xfff1
#define SHN_COMMON                                0xfff2
#define SHN_HIRESERVE                             0xffff

/*
 * Operating system and ABI
 */

#define ELFOSABI_SYSV                              0
#define ELFOSABI_HPUX                              1
#define ELFOSABI_STANDALONE                       255

/*
 * Type
 */

#define ET_NONE                                    0
#define ET_REL                                     1
#define ET_EXEC                                    2
#define ET_DYN                                     3
#define ET_CORE                                    4
#define ET_LOOS                                   0xfe00
#define ET_HIOS                                   0xfeff
#define ET_LOPROC                                 0xff00
#define ET_HIPROC                                 0xffff

/*
 * Machine
 */

#define EM_NONE                                    0
#define EM_M32                                     1
#define EM_SPARC                                   2
#define EM_386                                     3
#define EM_68K                                     4
#define EM_88K                                     5
#define EM_860                                     6
#define EM_MIPS                                    7
#define EM_X86_64                                 62

/*
 * Version
 */

#define EV_NONE                                    0
#define EV_CURRENT                                 1

/*
 * Section types
 */

#define SHT_NULL                                   0
#define SHT_PROGBITS                               1
#define SHT_SYMTAB                                 2
#define SHT_STRTAB                                 3
#define SHT_RELA                                   4
#define SHT_HASH                                   5
#define SHT_DYNAMIC                                6
#define SHT_NOTE                                   7
#define SHT_NOBITS                                 8
#define SHT_REL                                   9
#define SHT_SHLIB                                 10
#define SHT_DYNSYM                                11
#define SHT_LOOS                                  0x60000000
#define SHT_HIOS                                  0x6fffffff
#define SHT_LOPROC                                0x70000000
#define SHT_HIPROC                                0x7fffffff
#define SHT_LOUSER                                0x80000000
#define SHT_HIUSER                                0xffffffff

/*
 * Section flags
 */

#define SHF_WRITE                                  1
#define SHF_ALLOC                                  2
#define SHF_EXECINSTR                              4
#define SHF_MASKOS                                0x0f000000
#define SHF_MASKPROC                              0xf0000000

/*
 * Symbol binding
 */

#define STB_LOCAL                                  0
#define STB_GLOBAL                                 1
#define STB_WEAK                                   2
#define STB_LOOS                                  10
#define STB_HIOS                                  12
#define STB_LOPROC                                13
#define STB_HIPROC                                15

/*
 * Symbol types
 */

#define STT_NOTYPE                                 0
#define STT_OBJECT                                 1
#define STT_FUNC                                   2
#define STT_SECTION                                3
#define STT_FILE                                   4
#define STT_LOOS                                  10
#define STT_HIOS                                  12
#define STT_LOPROC                                13
#define STT_HIPROC                                14

/*
 * Relocation types
 */

#define R_386_NONE                                 0
#define R_386_32                                   1
#define R_386_PC32                                 2
#define R_386_GOT32                                3
#define R_386_PLT32                                4
#define R_386_COPY                                 5
#define R_386_GLOB_DAT                             6
#define R_386_JMP_SLOT                             7
#define R_386_RELATIVE                             8
#define R_386_GOTOFF                               9
#define R_386_GOTPC                               10

/*
 * Segment types
 */

#define PT_NULL                                    0
#define PT_LOAD                                    1
#define PT_DYNAMIC                                 2
#define PT_INTERP                                  3
#define PT_NOTE                                    4
#define PT_SHLIB                                   5
#define PT_PHDR                                    6
#define PT_LOOS                                   0x60000000
#define PT_HIOS                                   0x6fffffff
#define PT_LOPROC                                 0x70000000
#define PT_HIPROC                                 0x7fffffff

/*
 * Segment flags
 */

#define PF_EXECINSTR                               1
#define PF_WRITE                                   2
#define PF_READ                                    4
#define PF_MASKOS                                 0x00ff0000
#define PF_MASKPROC                               0xff000000

/*
 * Dynamic array tags
 */

#define DT_NULL                                    0
#define DT_NEEDED                                  1
#define DT_PLTRELSZ                                2
#define DT_PLTGOT                                  3
#define DT_HASH                                    4
#define DT_STRTAB                                  5
#define DT_SYMTAB                                  6
#define DT_RELA                                    7
#define DT_RELASZ                                  8
#define DT_RELAENT                                 9
#define DT_STRSZ                                  10
#define DT_SYMENT                                 11
#define DT_INIT                                   12
#define DT_FINI                                   13
#define DT_SONAME                                 14
#define DT_RPATH                                  15
#define DT_SYMBOLIC                               16
#define DT_REL                                    17
#define DT_RELSZ                                  18
#define DT_RELENT                                 19
#define DT_PLTREL                                 20
#define DT_DEBUG                                  21
#define DT_TEXTREL                                22
#define DT_JMPREL                                 23
#define DT_BIND_NOW                               24
#define DT_INIT_ARRAY                             25
#define DT_FINI_ARRAY                             26
#define DT_INIT_ARRAYSZ                           27
#define DT_FINI_ARRAYSZ                           28
#define DT_LOOS                                   0x60000000
#define DT_HIOS                                   0x6fffffff
#define DT_LOPROC                                 0x70000000
#define DT_HIPROC                                 0x7fffffff

struct elf_hdr {
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
} __attribute__((packed));

bool elf_check_file(struct elf_hdr *elf_hdr);

#endif /* __BOOT_ELF_H__ */