#ifndef __BOOT_ELF_H__
#define __BOOT_ELF_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <elfboot/linkage.h>

#define ELF_SYMTAB	".symtab"
#define ELF_STRTAB	".strtab"

/* 32-bit ELF base types. */
typedef uint32_t	Elf32_Addr;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Off;
typedef	 int32_t	Elf32_Sword;
typedef uint32_t	Elf32_Word;

/* 64-bit ELF base types. */
typedef uint64_t	Elf64_Addr;
typedef uint16_t	Elf64_Half;
typedef	 int16_t	Elf64_SHalf;
typedef uint64_t	Elf64_Off;
typedef	 int32_t	Elf64_Sword;
typedef uint32_t	Elf64_Word;
typedef uint64_t	Elf64_Xword;
typedef	 int64_t	Elf64_Sxword;

/* These constants are for the segment types stored in the image headers */
#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4
#define PT_SHLIB	5
#define PT_PHDR		6
#define PT_TLS		7
#define PT_LOOS		0x60000000
#define PT_HIOS		0x6fffffff
#define PT_LOPROC	0x70000000
#define PT_HIPROC	0x7fffffff

#define PN_XNUM 0xffff

/*
 * Elf file types
 */

#define ET_NONE		0
#define ET_REL		1
#define ET_EXEC		2
#define ET_DYN		3
#define ET_CORE		4
#define ET_LOPROC	0xff00
#define ET_HIPROC	0xffff

/*
 * Machine
 */

#define EM_NONE		0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
#define EM_88K		5
#define EM_860		6
#define EM_MIPS		7
#define EM_X86_64	62

/*
 * Dynamic sections
 */

#define DT_NULL			0
#define DT_NEEDED		1
#define DT_PLTRELSZ		2
#define DT_PLTGOT		3
#define DT_HASH			4
#define DT_STRTAB		5
#define DT_SYMTAB		6
#define DT_RELA			7
#define DT_RELASZ		8
#define DT_RELAENT		9
#define DT_STRSZ		10
#define DT_SYMENT		11
#define DT_INIT			12
#define DT_FINI			13
#define DT_SONAME		14
#define DT_RPATH 		15
#define DT_SYMBOLIC		16
#define DT_REL			17
#define DT_RELSZ		18
#define DT_RELENT		19
#define DT_PLTREL		20
#define DT_DEBUG		21
#define DT_TEXTREL		22
#define DT_JMPREL		23
#define DT_ENCODING		32
#define OLD_DT_LOOS		0x60000000
#define DT_LOOS			0x6000000d
#define DT_HIOS			0x6ffff000
#define DT_VALRNGLO		0x6ffffd00
#define DT_VALRNGHI		0x6ffffdff
#define DT_ADDRRNGLO	0x6ffffe00
#define DT_ADDRRNGHI	0x6ffffeff
#define DT_VERSYM		0x6ffffff0
#define DT_RELACOUNT	0x6ffffff9
#define DT_RELCOUNT		0x6ffffffa
#define DT_FLAGS_1		0x6ffffffb
#define DT_VERDEF		0x6ffffffc
#define	DT_VERDEFNUM	0x6ffffffd
#define DT_VERNEED		0x6ffffffe
#define	DT_VERNEEDNUM	0x6fffffff
#define OLD_DT_HIOS		0x6fffffff
#define DT_LOPROC		0x70000000
#define DT_HIPROC		0x7fffffff

/*
 * Symbol table
 */

#define STB_LOCAL		0
#define STB_GLOBAL		1
#define STB_WEAK		2

#define STT_NOTYPE		0
#define STT_OBJECT		1
#define STT_FUNC		2
#define STT_SECTION		3
#define STT_FILE		4
#define STT_COMMON		5
#define STT_TLS			6

#define ELF_ST_BIND(x)		((x) >> 4)
#define ELF_ST_TYPE(x)		(((unsigned int) x) & 0xf)
#define ELF32_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x)	ELF_ST_TYPE(x)
#define ELF64_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x)	ELF_ST_TYPE(x)

typedef struct dynamic {
	Elf32_Sword d_tag;
	union {
		Elf32_Sword	d_val;
		Elf32_Addr	d_ptr;
	} d_un;
} Elf32_Dyn;

typedef struct {
	Elf64_Sxword d_tag;
	union {
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
} Elf64_Dyn;

/*
 * Relocations
 */

#define ELF32_R_SYM(x)		((x) >> 8)
#define ELF32_R_TYPE(x)		((x) & 0xff)

#define ELF64_R_SYM(i)		((i) >> 32)
#define ELF64_R_TYPE(i)		((i) & 0xffffffff)

#define R_386_NONE		0
#define R_386_32		1
#define R_386_PC32		2
#define R_386_GOT32		3
#define R_386_PLT32		4
#define R_386_COPY		5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF	9
#define R_386_GOTPC		10
#define R_386_NUM		11

typedef struct elf32_rel {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;

typedef struct elf64_rel {
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
} Elf64_Rel;

typedef struct elf32_rela {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
} Elf32_Rela;

typedef struct elf64_rela {
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
	Elf64_Sxword r_addend;
} Elf64_Rela;

typedef struct elf32_sym {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} Elf32_Sym;

typedef struct elf64_sym {
	Elf64_Word st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf64_Half st_shndx;
	Elf64_Addr st_value;
	Elf64_Xword st_size;
} Elf64_Sym;


#define EI_NIDENT	16

typedef struct elf32_hdr {
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr {
	unsigned char	e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/*
 * Section permissions
 */

#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1

typedef struct elf32_phdr {
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr;

typedef struct elf64_phdr {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

/*
 * Section header types
 */

#define SHT_NULL		0
#define SHT_PROGBITS	1
#define SHT_SYMTAB		2
#define SHT_STRTAB		3
#define SHT_RELA		4
#define SHT_HASH		5
#define SHT_DYNAMIC		6
#define SHT_NOTE		7
#define SHT_NOBITS		8
#define SHT_REL			9
#define SHT_SHLIB		10
#define SHT_DYNSYM		11
#define SHT_NUM			12
#define SHT_LOPROC		0x70000000
#define SHT_HIPROC		0x7fffffff
#define SHT_LOUSER		0x80000000
#define SHT_HIUSER		0xffffffff

/*
 * Section header flags
 */

#define SHF_WRITE			0x1
#define SHF_ALLOC			0x2
#define SHF_EXECINSTR		0x4
#define SHF_RELA_LIVEPATCH	0x00100000
#define SHF_RO_AFTER_INIT	0x00200000
#define SHF_MASKPROC		0xf0000000

/*
 * Section header indexes
 */

#define SHN_UNDEF		0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC		0xff00
#define SHN_HIPROC		0xff1f
#define SHN_LIVEPATCH	0xff20
#define SHN_ABS			0xfff1
#define SHN_COMMON		0xfff2
#define SHN_HIRESERVE	0xffff
 
typedef struct elf32_shdr {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr;

typedef struct elf64_shdr {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
} Elf64_Shdr;

#define	EI_MAG0			0
#define	EI_MAG1			1
#define	EI_MAG2			2
#define	EI_MAG3			3
#define	EI_CLASS		4
#define	EI_DATA			5
#define	EI_VERSION		6
#define	EI_OSABI		7
#define	EI_PAD			8

#define	ELFMAG0			0x7f
#define	ELFMAG1			'E'
#define	ELFMAG2			'L'
#define	ELFMAG3			'F'
#define	ELFMAG			"\177ELF"
#define	SELFMAG			4

#define	ELFCLASSNONE	0
#define	ELFCLASS32		1
#define	ELFCLASS64		2
#define	ELFCLASSNUM		3

#define ELFDATANONE		0
#define ELFDATA2LSB		1
#define ELFDATA2MSB		2

#define EV_NONE			0
#define EV_CURRENT		1
#define EV_NUM			2

#define ELFOSABI_NONE	0
#define ELFOSABI_LINUX	3

#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif

/*
 * Notes used in ET_CORE. Architectures export some of the arch register sets
 * using the corresponding note types via the PTRACE_GETREGSET and
 * PTRACE_SETREGSET requests.
 */
#define NT_PRSTATUS		1
#define NT_PRFPREG		2
#define NT_PRPSINFO		3
#define NT_TASKSTRUCT	4
#define NT_AUXV			6
/*
 * Note to userspace developers: size of NT_SIGINFO note may increase
 * in the future to accomodate more fields, don't assume it is fixed!
 */
#define NT_SIGINFO			0x53494749
#define NT_FILE				0x46494c45
#define NT_PRXFPREG		 	0x46e62b7f
#define NT_PPC_VMX			0x100
#define NT_PPC_SPE			0x101
#define NT_PPC_VSX			0x102
#define NT_PPC_TAR			0x103
#define NT_PPC_PPR			0x104
#define NT_PPC_DSCR			0x105
#define NT_PPC_EBB			0x106
#define NT_PPC_PMU			0x107
#define NT_PPC_TM_CGPR		0x108
#define NT_PPC_TM_CFPR		0x109
#define NT_PPC_TM_CVMX		0x10a
#define NT_PPC_TM_CVSX		0x10b
#define NT_PPC_TM_SPR		0x10c
#define NT_PPC_TM_CTAR		0x10d
#define NT_PPC_TM_CPPR		0x10e
#define NT_PPC_TM_CDSCR		0x10f
#define NT_PPC_PKEY			0x110
#define NT_386_TLS			0x200
#define NT_386_IOPERM		0x201
#define NT_X86_XSTATE		0x202
#define NT_S390_HIGH_GPRS	0x300
#define NT_S390_TIMER		0x301
#define NT_S390_TODCMP		0x302
#define NT_S390_TODPREG		0x303
#define NT_S390_CTRS		0x304
#define NT_S390_PREFIX		0x305
#define NT_S390_LAST_BREAK	0x306
#define NT_S390_SYSTEM_CALL	0x307
#define NT_S390_TDB			0x308
#define NT_S390_VXRS_LOW	0x309
#define NT_S390_VXRS_HIGH	0x30a
#define NT_S390_GS_CB		0x30b
#define NT_S390_GS_BC		0x30c
#define NT_S390_RI_CB		0x30d
#define NT_ARM_VFP			0x400
#define NT_ARM_TLS			0x401
#define NT_ARM_HW_BREAK		0x402
#define NT_ARM_HW_WATCH		0x403
#define NT_ARM_SYSTEM_CALL	0x404
#define NT_ARM_SVE			0x405
#define NT_ARM_PAC_MASK		0x406
#define NT_ARM_PACA_KEYS	0x407
#define NT_ARM_PACG_KEYS	0x408
#define NT_ARC_V2			0x600
#define NT_VMCOREDD			0x700
#define NT_MIPS_DSP			0x800
#define NT_MIPS_FP_MODE		0x801
#define NT_MIPS_MSA			0x802

/* Note header in a PT_NOTE section */
typedef struct elf32_note {
	Elf32_Word	n_namesz;
	Elf32_Word	n_descsz;
	Elf32_Word	n_type;
} Elf32_Nhdr;

/* Note header in a PT_NOTE section */
typedef struct elf64_note {
	Elf64_Word n_namesz;
	Elf64_Word n_descsz;
	Elf64_Word n_type;
} Elf64_Nhdr;

bool is_elf_file(Elf32_Ehdr *ehdr);

bool is_elf_sane(Elf32_Ehdr *ehdr);

Elf32_Shdr *elf32_get_shdr(Elf32_Ehdr *ehdr);

Elf32_Shdr *elf32_get_shdr_shndx(Elf32_Ehdr *ehdr, Elf32_Half shndx);

void *elf32_section_addr(Elf32_Ehdr *ehdr, Elf32_Half shndx);

char *elf32_section_name(Elf32_Ehdr *ehdr, Elf32_Word shname);

void *elf32_find_section(Elf32_Ehdr *ehdr, const char *name, uint32_t *entnum);

#endif /* __BOOT_ELF_H__ */