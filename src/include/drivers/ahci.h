#ifndef __DRIVER_SATA_H__
#define __DRIVER_SATA_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

#include <drivers/ata.h>

#define DRIVER_AHCI		"AHCI"

#define AHCI_MAX_DEVICES	32

#define SATA_SIG_ATA	0x00000101
#define SATA_SIG_ATAPI	0xeb140101
#define SATA_SIG_SEMB	0xc33c0101
#define SATA_SIG_PM		0x96690101

struct hba_prdt {
	uint32_t dba;
	uint32_t dbau;
	uint32_t __reserved;
	uint32_t idbc;
} __packed;

#define HBA_IDBC_DBC(val)	(((val) & 0x3fffff) << 0)
#define HBA_IDBC_I(val)		(((val) & 0x01) << 31)

#define HBA_IDBC(dbc, i)	\
	HBA_IDBC_DBC(dbc) | HBA_IDBC_I(i)

struct hba_cmd_hdr {
	uint8_t pwacfl;
	uint8_t pmpcbr;
	uint16_t prdtl;
	uint32_t prdbc;
	uint32_t ctba;
	uint32_t ctbau;
	uint32_t __reserved[4];
} __packed;

#define HBA_PWACFL_CFL(val)	(((val) & 0x1f) << 0)
#define HBA_PWACFL_A(val)	(((val) & 0x01) << 5)
#define HBA_PWACFL_W(val)	(((val) & 0x01) << 6)
#define HBA_PWACFL_P(val)	(((val) & 0x01) << 7)

#define HBA_PWACFL(cfl, a, w, p)	\
	HBA_PWACFL_CFL(cfl) | HBA_PWACFL_A(a) | HBA_PWACFL_W(w) | HBA_PWACFL_P(p)

#define HBA_PMPCBR_R(val)	(((val) & 0x01) << 0)
#define HBA_PMPCBR_B(val)	(((val) & 0x01) << 1)
#define HBA_PMPCBR_C(val)	(((val) & 0x01) << 2)
#define HBA_PMPCBR_PMP(val)	(((val) & 0x0f) << 4)

struct hba_cmd_tbl {
	uint8_t cfis[64];
	uint8_t acmd[16];
	uint8_t __reserved[48];
	struct hba_prdt prdt[0];
} __packed;

/*
 * HBA FIS structures
 */

struct hba_fis_dma {
	uint8_t type;
	uint8_t pmdia;
	uint16_t __reserved1;
	uint64_t dma_buf_id;
	uint32_t __reserved2;
	uint32_t dma_buf_offset;
	uint32_t transfer_count;
	uint32_t __reserved3;
} __packed;

struct hba_fis_pio {
	uint8_t type;
	uint8_t pmdi;
	uint8_t status;
	uint8_t error;
	uint8_t lba_low;
	uint8_t lba_mid;
	uint8_t lba_high;
	uint8_t device;
	uint8_t lba_ext_low;
	uint8_t lba_ext_mid;
	uint8_t lba_ext_high;
	uint8_t __reserved1;
	uint8_t countl;
	uint8_t counth;
	uint8_t __reserved2;
	uint8_t e_status;
	uint16_t tc;
	uint16_t __reserved3;
} __packed;

struct hba_fis_reg {
	uint8_t type;
	uint8_t pmic;
	union {
		uint8_t status;
		uint8_t command;
	};
	union {
		uint8_t error;
		uint8_t feature;
	};
	uint8_t lba_low;
	uint8_t lba_mid;
	uint8_t lba_high;
	uint8_t device;
	uint8_t lba_ext_low;
	uint8_t lba_ext_mid;
	uint8_t lba_ext_high;
	uint8_t __reserved1;
	uint8_t countl;
	uint8_t counth;
	uint8_t __reserved2;
	uint8_t control;
	uint32_t __reserved3;
} __packed;

#define HBA_PMIC_PMP(val)	(((val) & 0x0f) << 0)
#define HBA_PMIC_I(val)		(((val) & 0x01) << 6)
#define HBA_PMIC_C(val)		(((val) & 0x01) << 7)

#define HBA_PMIC(pmp, i, c)	\
	HBA_PMIC_PMP(pmp) | HBA_PMIC_I(i) | HBA_PMIC_C(c)

#define FIS_REG_DWORD_SIZE sizeof(struct hba_fis_reg) / sizeof(uint32_t)

struct hba_fis {
	struct hba_fis_dma dma;
	uint32_t __reserved1[1];
	struct hba_fis_pio pio;
	uint32_t __reserved2[3];
	struct hba_fis_reg reg;
	uint32_t __reserved3[1];
	uint8_t sdbfis[8];
	uint8_t ufis[64];
	uint32_t __reserved4[24];
} __packed;

#define FIS_TYPE_REG_H2D	0x27
#define FIS_TYPE_REG_D2H	0x34
#define FIS_TYPE_DMA_ACT	0x29
#define FIS_TYPE_DMA_SETUP	0x41
#define FIS_TYPE_DATA		0x26
#define FIS_TYPE_BIST		0x58
#define FIS_TYPE_PIO_SETUP	0x2f
#define FIS_TYPE_DEV_BITS	0xa1

struct hba_port {
	uint32_t clb;
	uint32_t clbu;
	uint32_t fb;
	uint32_t fbu;
	uint32_t is;
	uint32_t ie;
	uint32_t cmd;
	uint32_t __reserved1;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sctl;
	uint32_t serr;
	uint32_t sact;
	uint32_t ci;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t devslp;
	uint32_t __reserved2[10];
	uint32_t vs[4];
} __packed;

#define HBA_PxIS_TFES	_BITUL(30)

#define HBA_PxCMD_ST	0x0001
#define HBA_PxCMD_FRE	0x0010
#define HBA_PxCMD_FR	0x4000
#define HBA_PxCMD_CR	0x8000

#define HBA_MEMORY_RESERVED_SIZE	(0x00A0 - 0x002C)
#define HBA_MEMORY_VENDOR_SIZE		(0x0100 - 0x00A0)

struct hba_memory {
	uint32_t cap;
	uint32_t ghc;
	uint32_t is;
	uint32_t pi;
	uint32_t vs;
	uint32_t ccc_ctl;
	uint32_t ccc_ports;
	uint32_t em_loc;
	uint32_t em_ctl;
	uint32_t cap2;
	uint32_t bohc;
	uint8_t __reserved[HBA_MEMORY_RESERVED_SIZE];
	uint8_t vendor[HBA_MEMORY_VENDOR_SIZE];
	struct hba_port ports[1];
} __packed;

#define HBA_CAP_NP(val)	(((val) >> 0) & 0x1f)

struct ahci_dev {
	int portno;
	volatile struct hba_port *port;
	uint8_t atapi;
	uint16_t *private;
};

struct ahci_cmd {
	uint8_t fiscmd;
	void *packet;
	size_t pkglen;
	void *buffer;
	size_t buflen;
	ata_regs_t reg;
};

#endif /* __DRIVER_SATA_H__ */