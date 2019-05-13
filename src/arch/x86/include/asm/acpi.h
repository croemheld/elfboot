#ifndef __X86_ACPI_H__
#define __X86_ACPI_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <asm/bda.h>

#include <elfboot/string.h>

#include <uapi/elfboot/common.h>

#define ACPI_EBDA_RANGE                           1024
#define ACPI_HMEM_BASE                            0xE0000
#define ACPI_HMEM_RANGE                           0x20000

#define ACPI_RSDP_SIGNATURE                       "RSD PTR "
#define ACPI_RSDP_CHECKSUM_LENGTH                 20
#define ACPI_RSDP_XCHECKSUM_LENGTH                36

struct rsdp_descriptor {
	char signature[8];
	uint8_t  checksum;
	char oem_id[6];
	uint8_t  revision;
	uint32_t rsdt_addr;

	/*
	 * Members below only for ACPI version >= 2.0
	 */
	
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t  ext_checksum;
	uint8_t  _reserved[3];
} __attribute__((packed));

struct acpi_sdt_hdr {
	char signature[4];
	uint32_t length;
	uint8_t  revision;
	uint8_t  checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdt {
	struct acpi_sdt_hdr acpi_hdr;
	uint32_t sdt_ptr[];
} __attribute__((packed));

struct acpi_xsdt {
	struct acpi_sdt_hdr acpi_hdr;
	uint64_t sdt_ptr[];
} __attribute__((packed));

#endif /* __X86_ACPI_H__ */