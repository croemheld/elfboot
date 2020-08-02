#ifndef __DRIVER_ATA_H__
#define __DRIVER_ATA_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>

/*
 * TODO CRO: This mess needs to be cleaned up!
 */

/*
 * ATA status registers
 */

#define ATA_SR_BSY                                0x80
#define ATA_SR_DRDY                               0x40
#define ATA_SR_DF                                 0x20
#define ATA_SR_DSC                                0x10
#define ATA_SR_DRQ                                0x08
#define ATA_SR_CORR                               0x04
#define ATA_SR_IDX                                0x02
#define ATA_SR_ERR                                0x01

#define ATA_SR_POLL	(ATA_SR_ERR | ATA_SR_DRQ | ATA_SR_DF)

/*
 * ATA error registers
 */

#define ATA_ER_BBK                                0x80
#define ATA_ER_UNC                                0x40
#define ATA_ER_MC                                 0x20
#define ATA_ER_IDNF                               0x10
#define ATA_ER_MCR                                0x08
#define ATA_ER_ABRT                               0x04
#define ATA_ER_TK0NF                              0x02
#define ATA_ER_AMNF                               0x01

/*
 * ATA commands
 */

#define ATA_CMD_READ_PIO                          0x20
#define ATA_CMD_READ_PIO_EXT                      0x24
#define ATA_CMD_READ_DMA                          0xC8
#define ATA_CMD_READ_DMA_EXT                      0x25
#define ATA_CMD_WRITE_PIO                         0x30
#define ATA_CMD_WRITE_PIO_EXT                     0x34
#define ATA_CMD_WRITE_DMA                         0xCA
#define ATA_CMD_WRITE_DMA_EXT                     0x35
#define ATA_CMD_CACHE_FLUSH                       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT                   0xEA
#define ATA_CMD_PACKET                            0xA0
#define ATA_CMD_IDENTIFY_PACKET                   0xA1
#define ATA_CMD_IDENTIFY                          0xEC

#define ATAPI_CMD_READ                            0xA8
#define ATAPI_CMD_EJECT                           0x1B

#define ATA_IDENT_DEVICETYPE                        0
#define ATA_IDENT_CYLINDERS                         2
#define ATA_IDENT_HEADS                             6
#define ATA_IDENT_SECTORS                          12
#define ATA_IDENT_SERIAL                           20
#define ATA_IDENT_MODEL                            54
#define ATA_IDENT_CAPABILITIES                     98
#define ATA_IDENT_FIELDVALID                      106
#define ATA_IDENT_MAX_LBA                         120
#define ATA_IDENT_COMMANDSETS                     164
#define ATA_IDENT_MAX_LBA_EXT                     200

#define IDE_ATA                                   0x00
#define IDE_ATAPI                                 0x01
 
#define ATA_MASTER                                0x00
#define ATA_SLAVE                                 0x01

/*
 * Additional ATA registers
 */

#define ATA_REG_DATA                               0
#define ATA_REG_ERROR                              1
#define ATA_REG_FEATURES                           1
#define ATA_REG_SECTORS                            2
#define ATAPI_REG_IREASON                          2
#define ATA_REG_LBALOW                             3
#define ATA_REG_LBAMID                             4
#define ATAPI_REG_CNTLOW                           4
#define ATA_REG_LBAHIGH                            5
#define ATAPI_REG_CNTHIGH                          5
#define ATA_REG_HDDEVSEL                           6
#define ATA_REG_COMMAND                            7
#define ATA_REG_STATUS                             7
#define ATA_REG_SECCOUNT1                          8
#define ATA_REG_LBA3                               9
#define ATA_REG_LBA4                              10
#define ATA_REG_LBA5                              11
#define ATA_REG_CONTROL                           12
#define ATA_REG_ALTSTATUS                         12
#define ATA_REG_DEVADDRESS                        13

#define ATAPI_IREASON_MASK                        0x3
#define ATAPI_IREASON_DATA_OUT                    0x0
#define ATAPI_IREASON_CMD_OUT                     0x1
#define ATAPI_IREASON_DATA_IN                     0x2
#define ATAPI_IREASON_ERROR                       0x3

/*
 * Channels
 */

#define ATA_PRIMARY                               0x00
#define ATA_SECONDARY                             0x01

/*
 * Directions
 */

#define ATA_COMMAND_READ                          0x00
#define ATA_COMMAND_WRITE                         0x01

/*
 * Logical Block Addressing
 */

#define ATA_LBA_PIO28_LIMIT                       0xFFFFFFFF

/*
 * Misc
 */

#define ATA_IO_TIMEOUT                            1000000
#define ATA_IDENTIFY_WORD_COUNT                   (ATA_IDENTIFY_BUFFER_SIZE / 2)

typedef union {
	uint8_t raw[11];
	struct {
		union {
			uint8_t features;
			uint8_t error;
		};
		union {
			uint8_t sectors;
			uint8_t atapi_ireason;
		};
		union {
			uint8_t lba_low;
			uint8_t sectnum;
		};
		union {
			uint8_t lba_mid;
			uint8_t cyllsb;
			uint8_t atapi_cntlow;
		};
		union {
			uint8_t lba_high;
			uint8_t cylmsb;
			uint8_t atapi_cnthigh;
		};
		uint8_t disk;
		union {
			uint8_t cmd;
			uint8_t status;
		};
		uint8_t sectors48;
		uint8_t lba48_low;
		uint8_t lba48_mid;
		uint8_t lba48_high;
	};

} ata_regs_t;

struct ata_cmd {
	void *buf;
	size_t bufsize;
	void *cmd;
	size_t cmdsize;
	int write;
	ata_regs_t reg;
} __packed;

/* -------------------------------------------------------------------------- */

struct ata_id {
/*000*/ uint16_t config;			/* configuration info */

#define ATA_PROTO_MASK			0x8003
#define ATA_PROTO_ATAPI			0x8000
#define ATA_PROTO_ATAPI_12		0x8000
#define ATA_PROTO_ATAPI_16		0x8001
#define ATA_PROTO_CFA			0x848a
#define ATA_ATAPI_TYPE_MASK		0x1f00
#define ATA_ATAPI_TYPE_DIRECT		0x0000	/* disk/floppy */
#define ATA_ATAPI_TYPE_TAPE		0x0100	/* streaming tape */
#define ATA_ATAPI_TYPE_CDROM		0x0500	/* CD-ROM device */
#define ATA_ATAPI_TYPE_OPTICAL		0x0700	/* optical disk */
#define ATA_DRQ_MASK			0x0060
#define ATA_DRQ_SLOW			0x0000	/* cpu 3 ms delay */
#define ATA_DRQ_INTR			0x0020	/* interrupt 10 ms delay */
#define ATA_DRQ_FAST			0x0040	/* accel 50 us delay */
#define ATA_RESP_INCOMPLETE		0x0004

/*001*/ uint16_t cylinders;			/* # of cylinders */
/*002*/ uint16_t specconf;			/* specific configuration */
/*003*/ uint16_t heads;				/* # heads */
	uint16_t obsolete4;
	uint16_t obsolete5;
/*006*/ uint16_t sectors;			/* # sectors/track */
/*007*/ uint16_t vendor7[3];
/*010*/ uint8_t  serial[20];			/* serial number */
/*020*/ uint16_t retired20;
	uint16_t retired21;
	uint16_t obsolete22;
/*023*/ uint8_t  revision[8];			/* firmware revision */
/*027*/ uint8_t  model[40];			/* model name */
/*047*/ uint16_t sectors_intr;			/* sectors per interrupt */
/*048*/ uint16_t tcg;				/* Trusted Computing Group */

#define ATA_SUPPORT_TCG			0x0001

/*049*/ uint16_t capabilities1;

#define ATA_SUPPORT_DMA			0x0100
#define ATA_SUPPORT_LBA			0x0200
#define ATA_SUPPORT_IORDYDIS		0x0400
#define ATA_SUPPORT_IORDY		0x0800
#define ATA_SUPPORT_OVERLAP		0x4000

/*050*/ uint16_t capabilities2;
/*051*/ uint16_t retired_piomode;		/* PIO modes 0-2 */

#define ATA_RETIRED_PIO_MASK		0x0300

/*052*/ uint16_t retired_dmamode;		/* DMA modes */

#define ATA_RETIRED_DMA_MASK		0x0003

/*053*/ uint16_t atavalid;			/* fields valid */

#define ATA_FLAG_54_58			0x0001	/* words 54-58 valid */
#define ATA_FLAG_64_70			0x0002	/* words 64-70 valid */
#define ATA_FLAG_88			0x0004	/* word 88 valid */

/*054*/ uint16_t current_cylinders;
/*055*/ uint16_t current_heads;
/*056*/ uint16_t current_sectors;
/*057*/ uint16_t current_size_1;
/*058*/ uint16_t current_size_2;
/*059*/ uint16_t multi;

#define ATA_SUPPORT_BLOCK_ERASE_EXT	0x8000
#define ATA_SUPPORT_OVERWRITE_EXT	0x4000
#define ATA_SUPPORT_CRYPTO_SCRAMBLE_EXT	0x2000
#define ATA_SUPPORT_SANITIZE		0x1000
#define ATA_MULTI_VALID			0x0100

/*060*/ uint32_t lba_size;
	uint16_t obsolete62;
/*063*/ uint16_t mwdmamodes;			/* multiword DMA modes */
/*064*/ uint16_t apiomodes;			/* advanced PIO modes */

/*065*/ uint16_t mwdmamin;			/* min. M/W DMA time/word ns */
/*066*/ uint16_t mwdmarec;			/* rec. M/W DMA time ns */
/*067*/ uint16_t pioblind;			/* min. PIO cycle w/o flow */
/*068*/ uint16_t pioiordy;			/* min. PIO cycle IORDY flow */
/*069*/ uint16_t support3;

#define ATA_SUPPORT_RZAT		0x0020
#define ATA_SUPPORT_DRAT		0x4000
#define ATA_ENCRYPTS_ALL_USER_DATA	0x0010	/* Self-encrypting drive */
#define	ATA_SUPPORT_ZONE_MASK		0x0003
#define	ATA_SUPPORT_ZONE_NR		0x0000
#define	ATA_SUPPORT_ZONE_HOST_AWARE	0x0001
#define	ATA_SUPPORT_ZONE_DEV_MANAGED	0x0002

	uint16_t reserved70;
/*071*/ uint16_t rlsovlap;			/* rel time (us) for overlap */
/*072*/ uint16_t rlsservice;			/* rel time (us) for service */
	uint16_t reserved73;
	uint16_t reserved74;
/*075*/ uint16_t queue;

#define ATA_QUEUE_LEN(x)		((x) & 0x001f)

/*76*/  uint16_t satacapabilities;

#define ATA_SATA_GEN1			0x0002
#define ATA_SATA_GEN2			0x0004
#define ATA_SATA_GEN3			0x0008
#define ATA_SUPPORT_NCQ			0x0100
#define ATA_SUPPORT_IFPWRMNGTRCV	0x0200
#define ATA_SUPPORT_PHYEVENTCNT		0x0400
#define ATA_SUPPORT_NCQ_UNLOAD		0x0800
#define ATA_SUPPORT_NCQ_PRIO		0x1000
#define ATA_SUPPORT_HAPST		0x2000
#define ATA_SUPPORT_DAPST		0x4000
#define ATA_SUPPORT_READLOGDMAEXT	0x8000

/*77*/  uint16_t satacapabilities2;

#define ATA_SATA_CURR_GEN_MASK		0x0006
#define ATA_SUPPORT_NCQ_STREAM		0x0010
#define ATA_SUPPORT_NCQ_QMANAGEMENT	0x0020
#define ATA_SUPPORT_RCVSND_FPDMA_QUEUED	0x0040

/*78*/  uint16_t satasupport;

#define ATA_SUPPORT_NONZERO             0x0002
#define ATA_SUPPORT_AUTOACTIVATE        0x0004
#define ATA_SUPPORT_IFPWRMNGT           0x0008
#define ATA_SUPPORT_INORDERDATA         0x0010
#define ATA_SUPPORT_ASYNCNOTIF          0x0020
#define ATA_SUPPORT_SOFTSETPRESERVE     0x0040

/*79*/  uint16_t sataenabled;

#define ATA_ENABLED_DAPST		0x0080

/*080*/ uint16_t version_major;
/*081*/ uint16_t version_minor;

	struct {
/*082/085*/ uint16_t command1;

#define ATA_SUPPORT_SMART		0x0001
#define ATA_SUPPORT_SECURITY		0x0002
#define ATA_SUPPORT_REMOVABLE		0x0004
#define ATA_SUPPORT_POWERMGT		0x0008
#define ATA_SUPPORT_PACKET		0x0010
#define ATA_SUPPORT_WRITECACHE		0x0020
#define ATA_SUPPORT_LOOKAHEAD		0x0040
#define ATA_SUPPORT_RELEASEIRQ		0x0080
#define ATA_SUPPORT_SERVICEIRQ		0x0100
#define ATA_SUPPORT_RESET		0x0200
#define ATA_SUPPORT_PROTECTED		0x0400
#define ATA_SUPPORT_WRITEBUFFER		0x1000
#define ATA_SUPPORT_READBUFFER		0x2000
#define ATA_SUPPORT_NOP			0x4000

/*083/086*/ uint16_t command2;

#define ATA_SUPPORT_MICROCODE		0x0001
#define ATA_SUPPORT_QUEUED		0x0002
#define ATA_SUPPORT_CFA			0x0004
#define ATA_SUPPORT_APM			0x0008
#define ATA_SUPPORT_NOTIFY		0x0010
#define ATA_SUPPORT_STANDBY		0x0020
#define ATA_SUPPORT_SPINUP		0x0040
#define ATA_SUPPORT_MAXSECURITY		0x0100
#define ATA_SUPPORT_AUTOACOUSTIC	0x0200
#define ATA_SUPPORT_ADDRESS48		0x0400
#define ATA_SUPPORT_OVERLAY		0x0800
#define ATA_SUPPORT_FLUSHCACHE		0x1000
#define ATA_SUPPORT_FLUSHCACHE48	0x2000

/*084/087*/ uint16_t extension;

#define ATA_SUPPORT_SMARTLOG		0x0001
#define ATA_SUPPORT_SMARTTEST		0x0002
#define ATA_SUPPORT_MEDIASN		0x0004
#define ATA_SUPPORT_MEDIAPASS		0x0008
#define ATA_SUPPORT_STREAMING		0x0010
#define ATA_SUPPORT_GENLOG		0x0020
#define ATA_SUPPORT_WRITEDMAFUAEXT	0x0040
#define ATA_SUPPORT_WRITEDMAQFUAEXT	0x0080
#define ATA_SUPPORT_64BITWWN		0x0100
#define ATA_SUPPORT_UNLOAD		0x2000

	} __packed support, enabled;

/*088*/ uint16_t udmamodes;			/* UltraDMA modes */
/*089*/ uint16_t erase_time;			/* time req'd in 2min units */
/*090*/ uint16_t enhanced_erase_time;		/* time req'd in 2min units */
/*091*/ uint16_t apm_value;
/*092*/ uint16_t master_passwd_revision;	/* password revision code */
/*093*/ uint16_t hwres;

#define ATA_CABLE_ID			0x2000

/*094*/ uint16_t acoustic;

#define ATA_ACOUSTIC_CURRENT(x)		((x) & 0x00ff)
#define ATA_ACOUSTIC_VENDOR(x)		(((x) & 0xff00) >> 8)

/*095*/ uint16_t stream_min_req_size;
/*096*/ uint16_t stream_transfer_time;
/*097*/ uint16_t stream_access_latency;
/*098*/ uint32_t stream_granularity;
/*100*/ uint64_t lba_size48;
	uint16_t reserved104;
/*105*/	uint16_t max_dsm_blocks;
/*106*/	uint16_t pss;

#define ATA_PSS_LSPPS			0x000F
#define ATA_PSS_LSSABOVE512		0x1000
#define ATA_PSS_MULTLS			0x2000
#define ATA_PSS_VALID_MASK		0xC000
#define ATA_PSS_VALID_VALUE		0x4000

/*107*/ uint16_t isd;
/*108*/ uint16_t wwn[4];
	uint16_t reserved112[5];
/*117*/ uint32_t lss;
/*119*/ uint16_t support2;

#define ATA_SUPPORT_WRITEREADVERIFY	0x0002
#define ATA_SUPPORT_WRITEUNCORREXT	0x0004
#define ATA_SUPPORT_RWLOGDMAEXT		0x0008
#define ATA_SUPPORT_MICROCODE3		0x0010
#define ATA_SUPPORT_FREEFALL		0x0020
#define ATA_SUPPORT_SENSE_REPORT	0x0040
#define ATA_SUPPORT_EPC			0x0080

/*120*/ uint16_t enabled2;

#define ATA_ENABLED_WRITEREADVERIFY	0x0002
#define ATA_ENABLED_WRITEUNCORREXT	0x0004
#define ATA_ENABLED_FREEFALL		0x0020
#define ATA_ENABLED_SENSE_REPORT	0x0040
#define ATA_ENABLED_EPC			0x0080

	uint16_t reserved121[6];
/*127*/ uint16_t removable_status;
/*128*/ uint16_t security_status;

#define ATA_SECURITY_LEVEL		0x0100	/* 0: high, 1: maximum */
#define ATA_SECURITY_ENH_SUPP		0x0020	/* enhanced erase supported */
#define ATA_SECURITY_COUNT_EXP		0x0010	/* count expired */
#define ATA_SECURITY_FROZEN		0x0008	/* security config is frozen */
#define ATA_SECURITY_LOCKED		0x0004	/* drive is locked */
#define ATA_SECURITY_ENABLED		0x0002	/* ATA Security is enabled */
#define ATA_SECURITY_SUPPORTED		0x0001	/* ATA Security is supported */

	uint16_t reserved129[31];
/*160*/ uint16_t cfa_powermode1;
	uint16_t reserved161;
/*162*/ uint16_t cfa_kms_support;
/*163*/ uint16_t cfa_trueide_modes;
/*164*/ uint16_t cfa_memory_modes;
	uint16_t reserved165[3];
/*168*/ uint16_t form_factor;

#define ATA_FORM_FACTOR_MASK		0x000f
#define ATA_FORM_FACTOR_NOT_REPORTED	0x0000
#define ATA_FORM_FACTOR_5_25		0x0001
#define ATA_FORM_FACTOR_3_5		0x0002
#define ATA_FORM_FACTOR_2_5		0x0003
#define ATA_FORM_FACTOR_1_8		0x0004
#define ATA_FORM_FACTOR_SUB_1_8		0x0005
#define ATA_FORM_FACTOR_MSATA		0x0006
#define ATA_FORM_FACTOR_M_2		0x0007
#define ATA_FORM_FACTOR_MICRO_SSD	0x0008
#define ATA_FORM_FACTOR_C_FAST		0x0009

/*169*/	uint16_t support_dsm;

#define ATA_SUPPORT_DSM_TRIM		0x0001

	uint16_t reserved170[6];
/*176*/ uint8_t  media_serial[60];
/*206*/ uint16_t sct;
	uint16_t reserved207[2];
/*209*/ uint16_t lsalign;
/*210*/ uint16_t wrv_sectors_m3_1;
	uint16_t wrv_sectors_m3_2;
/*212*/ uint16_t wrv_sectors_m2_1;
	uint16_t wrv_sectors_m2_2;
/*214*/ uint16_t nv_cache_caps;
/*215*/ uint16_t nv_cache_size_1;
	uint16_t nv_cache_size_2;
/*217*/ uint16_t media_rotation_rate;

#define ATA_RATE_NOT_REPORTED		0x0000
#define ATA_RATE_NON_ROTATING		0x0001

	uint16_t reserved218;
/*219*/ uint16_t nv_cache_opt;
/*220*/ uint16_t wrv_mode;
	uint16_t reserved221;
/*222*/ uint16_t transport_major;
/*223*/ uint16_t transport_minor;
	uint16_t reserved224[31];
/*255*/ uint16_t integrity;
} __packed;

#define ATA_DEV_MASTER		0x00
#define ATA_DEV_SLAVE		0x10
#define ATA_DEV_LBA		0x40

/* ATA limits */
#define ATA_MAX_28BIT_LBA	268435455UL

/* ATA Status Register */
#define ATA_STATUS_ERROR		0x01
#define ATA_STATUS_SENSE_AVAIL		0x02
#define ATA_STATUS_ALIGN_ERR		0x04
#define ATA_STATUS_DATA_REQ		0x08
#define ATA_STATUS_DEF_WRITE_ERR	0x10
#define ATA_STATUS_DEVICE_FAULT		0x20
#define ATA_STATUS_DEVICE_READY		0x40
#define ATA_STATUS_BUSY			0x80

/* ATA Error Register */
#define ATA_ERROR_ABORT		0x04
#define ATA_ERROR_ID_NOT_FOUND	0x10


/* ATA commands */
#define ATA_NOP				0x00	/* NOP */
#define ATA_NF_FLUSHQUEUE		0x00	/* flush queued cmd's */
#define ATA_NF_AUTOPOLL			0x01	/* start autopoll function */
#define ATA_DATA_SET_MANAGEMENT		0x06
#define ATA_DSM_TRIM			0x01
#define ATA_DEVICE_RESET		0x08	/* reset device */
#define ATA_READ			0x20	/* read */
#define ATA_READ48			0x24	/* read 48bit LBA */
#define ATA_READ_DMA48			0x25	/* read DMA 48bit LBA */
#define ATA_READ_DMA_QUEUED48		0x26	/* read DMA QUEUED 48bit LBA */
#define ATA_READ_NATIVE_MAX_ADDRESS48	0x27	/* read native max addr 48bit */
#define ATA_READ_MUL48			0x29	/* read multi 48bit LBA */
#define ATA_READ_STREAM_DMA48		0x2A	/* read DMA stream 48bit LBA */
#define ATA_READ_LOG_EXT		0x2F	/* read log ext - PIO Data-In */
#define ATA_READ_STREAM48		0x2B	/* read stream 48bit LBA */
#define ATA_WRITE			0x30	/* write */
#define ATA_WRITE48			0x34	/* write 48bit LBA */
#define ATA_WRITE_DMA48			0x35	/* write DMA 48bit LBA */
#define ATA_WRITE_DMA_QUEUED48		0x36	/* write DMA QUEUED 48bit LBA*/
#define ATA_SET_MAX_ADDRESS48		0x37	/* set max address 48bit */
#define ATA_WRITE_MUL48			0x39	/* write multi 48bit LBA */
#define ATA_WRITE_STREAM_DMA48		0x3A
#define ATA_WRITE_STREAM48		0x3B
#define ATA_WRITE_DMA_FUA48		0x3D
#define ATA_WRITE_DMA_QUEUED_FUA48	0x3E
#define ATA_WRITE_LOG_EXT		0x3F
#define ATA_READ_VERIFY			0x40
#define ATA_READ_VERIFY48		0x42
#define ATA_WRITE_UNCORRECTABLE48	0x45	/* write uncorrectable 48bit LBA */
#define ATA_WU_PSEUDO			0x55	/* pseudo-uncorrectable error */
#define ATA_WU_FLAGGED			0xAA	/* flagged-uncorrectable error */
#define ATA_READ_LOG_DMA_EXT		0x47	/* read log DMA ext - PIO Data-In */
#define	ATA_ZAC_MANAGEMENT_IN		0x4A	/* ZAC management in */
#define	ATA_ZM_REPORT_ZONES		0x00	/* report zones */
#define	ATA_WRITE_LOG_DMA_EXT		0x57	/* WRITE LOG DMA EXT */
#define	ATA_TRUSTED_NON_DATA		0x5B	/* TRUSTED NON-DATA */
#define	ATA_TRUSTED_RECEIVE		0x5C	/* TRUSTED RECEIVE */
#define	ATA_TRUSTED_RECEIVE_DMA		0x5D	/* TRUSTED RECEIVE DMA */
#define	ATA_TRUSTED_SEND		0x5E	/* TRUSTED SEND */
#define	ATA_TRUSTED_SEND_DMA		0x5F	/* TRUSTED SEND DMA */
#define ATA_READ_FPDMA_QUEUED		0x60	/* read DMA NCQ */
#define ATA_WRITE_FPDMA_QUEUED		0x61	/* write DMA NCQ */
#define ATA_NCQ_NON_DATA		0x63	/* NCQ non-data command */
#define	ATA_ABORT_NCQ_QUEUE		0x00	/* abort NCQ queue */
#define	ATA_DEADLINE_HANDLING		0x01	/* deadline handling */
#define	ATA_SET_FEATURES		0x05	/* set features */
#define	ATA_ZERO_EXT			0x06	/* zero ext */
#define	ATA_NCQ_ZAC_MGMT_OUT		0x07	/* NCQ ZAC mgmt out no data */
#define ATA_SEND_FPDMA_QUEUED		0x64	/* send DMA NCQ */
#define	ATA_SFPDMA_DSM			0x00	/* Data set management */
#define	ATA_SFPDMA_DSM_TRIM		0x01	/* Set trim bit in auxiliary */
#define	ATA_SFPDMA_HYBRID_EVICT		0x01	/* Hybrid Evict */
#define	ATA_SFPDMA_WLDMA		0x02	/* Write Log DMA EXT */
#define	ATA_SFPDMA_ZAC_MGMT_OUT		0x03	/* NCQ ZAC mgmt out w/data */
#define ATA_RECV_FPDMA_QUEUED		0x65	/* receive DMA NCQ */
#define	ATA_RFPDMA_RL_DMA_EXT		0x00	/* Read Log DMA EXT */
#define	ATA_RFPDMA_ZAC_MGMT_IN		0x02	/* NCQ ZAC mgmt in w/data */
#define ATA_SEP_ATTN			0x67	/* SEP request */
#define ATA_SEEK			0x70	/* seek */
#define	ATA_ZAC_MANAGEMENT_OUT		0x9F	/* ZAC management out */
#define	ATA_ZM_CLOSE_ZONE		0x01	/* close zone */
#define	ATA_ZM_FINISH_ZONE		0x02	/* finish zone */
#define	ATA_ZM_OPEN_ZONE		0x03	/* open zone */
#define	ATA_ZM_RWP			0x04	/* reset write pointer */
#define	ATA_DOWNLOAD_MICROCODE		0x92	/* DOWNLOAD MICROCODE */
#define	ATA_DOWNLOAD_MICROCODE_DMA	0x93	/* DOWNLOAD MICROCODE DMA */
#define ATA_PACKET_CMD			0xA0	/* packet command */
#define ATA_ATAPI_IDENTIFY		0xA1	/* get ATAPI params*/
#define ATA_SERVICE			0xA2	/* service command */
#define ATA_SMART_CMD			0xB0	/* SMART command */
#define ATA_CFA_ERASE			0xC0	/* CFA erase */
#define ATA_READ_MUL			0xC4	/* read multi */
#define ATA_WRITE_MUL			0xC5	/* write multi */
#define ATA_SET_MULTI			0xC6	/* set multi size */
#define ATA_READ_DMA_QUEUED		0xC7	/* read DMA QUEUED */
#define ATA_READ_DMA			0xC8	/* read DMA */
#define ATA_WRITE_DMA			0xCA	/* write DMA */
#define ATA_WRITE_DMA_QUEUED		0xCC	/* write DMA QUEUED */
#define ATA_WRITE_MUL_FUA48		0xCE
#define ATA_STANDBY_IMMEDIATE		0xE0	/* standby immediate */
#define ATA_IDLE_IMMEDIATE		0xE1	/* idle immediate */
#define ATA_STANDBY_CMD			0xE2	/* standby */
#define ATA_IDLE_CMD			0xE3	/* idle */
#define ATA_READ_BUFFER			0xE4	/* read buffer */
#define ATA_READ_PM			0xE4	/* read portmultiplier */
#define ATA_CHECK_POWER_MODE		0xE5	/* device power mode */
#define ATA_SLEEP			0xE6	/* sleep */
#define ATA_FLUSHCACHE			0xE7	/* flush cache to disk */
#define	ATA_WRITE_BUFFER		0xE8	/* write buffer */
#define ATA_WRITE_PM			0xE8	/* write portmultiplier */
#define	ATA_READ_BUFFER_DMA		0xE9	/* read buffer DMA */
#define ATA_FLUSHCACHE48		0xEA	/* flush cache to disk */
#define	ATA_WRITE_BUFFER_DMA		0xEB	/* write buffer DMA */
#define ATA_ATA_IDENTIFY		0xEC	/* get ATA params */
#define ATA_SETFEATURES			0xEF	/* features command */
#define ATA_SF_ENAB_WCACHE		0x02	/* enable write cache */
#define ATA_SF_DIS_WCACHE		0x82	/* disable write cache */
#define ATA_SF_SETXFER			0x03	/* set transfer mode */
#define	ATA_SF_APM			0x05	/* Enable APM feature set */
#define ATA_SF_ENAB_PUIS		0x06	/* enable PUIS */
#define ATA_SF_DIS_PUIS			0x86	/* disable PUIS */
#define ATA_SF_PUIS_SPINUP		0x07	/* PUIS spin-up */
#define	ATA_SF_WRV			0x0B	/* Enable Write-Read-Verify */
#define ATA_SF_DLC			0x0C	/* Enable device life control */
#define ATA_SF_SATA			0x10	/* Enable use of SATA feature */
#define ATA_SF_FFC			0x41	/* Free-fall Control */
#define ATA_SF_MHIST			0x43	/* Set Max Host Sect. Times */
#define ATA_SF_RATE			0x45	/* Set Rate Basis */
#define ATA_SF_EPC			0x4A	/* Extended Power Conditions */
#define ATA_SF_ENAB_RCACHE		0xAA	/* enable readahead cache */
#define ATA_SF_DIS_RCACHE		0x55	/* disable readahead cache */
#define ATA_SF_ENAB_RELIRQ		0x5D	/* enable release interrupt */
#define ATA_SF_DIS_RELIRQ		0xDD	/* disable release interrupt */
#define ATA_SF_ENAB_SRVIRQ		0x5E	/* enable service interrupt */
#define ATA_SF_DIS_SRVIRQ		0xDE	/* disable service interrupt */
#define ATA_SF_LPSAERC			0x62	/* Long Phys Sect Align ErrRep*/
#define ATA_SF_DSN			0x63	/* Device Stats Notification */
#define ATA_CHECK_POWER_MODE		0xE5	/* Check Power Mode */
#define ATA_SECURITY_SET_PASSWORD	0xF1	/* set drive password */
#define ATA_SECURITY_UNLOCK		0xF2	/* unlock drive using passwd */
#define ATA_SECURITY_ERASE_PREPARE	0xF3	/* prepare to erase drive */
#define ATA_SECURITY_ERASE_UNIT		0xF4	/* erase all blocks on drive */
#define ATA_SECURITY_FREEZE_LOCK	0xF5	/* freeze security config */
#define ATA_SECURITY_DISABLE_PASSWORD	0xF6	/* disable drive password */
#define ATA_READ_NATIVE_MAX_ADDRESS	0xF8	/* read native max address */
#define ATA_SET_MAX_ADDRESS		0xF9	/* set max address */


/* ATAPI commands */
#define ATAPI_TEST_UNIT_READY		0x00	/* check if device is ready */
#define ATAPI_REZERO			0x01	/* rewind */
#define ATAPI_REQUEST_SENSE		0x03	/* get sense data */
#define ATAPI_FORMAT			0x04	/* format unit */
#define ATAPI_READ			0x08	/* read data */
#define ATAPI_WRITE			0x0A	/* write data */
#define ATAPI_WEOF			0x10	/* write filemark */
#define ATAPI_WF_WRITE			0x01
#define ATAPI_SPACE			0x11	/* space command */
#define ATAPI_SP_FM			0x01
#define ATAPI_SP_EOD			0x03
#define ATAPI_INQUIRY			0x12	/* get inquiry data */
#define ATAPI_MODE_SELECT		0x15	/* mode select */
#define ATAPI_ERASE			0x19	/* erase */
#define ATAPI_MODE_SENSE		0x1A	/* mode sense */
#define ATAPI_START_STOP		0x1B	/* start/stop unit */
#define ATAPI_SS_LOAD			0x01
#define ATAPI_SS_RETENSION		0x02
#define ATAPI_SS_EJECT			0x04
#define ATAPI_PREVENT_ALLOW		0x1E	/* media removal */
#define ATAPI_READ_FORMAT_CAPACITIES	0x23	/* get format capacities */
#define ATAPI_READ_CAPACITY		0x25	/* get volume capacity */
#define ATAPI_READ_BIG			0x28	/* read data */
#define ATAPI_WRITE_BIG			0x2A	/* write data */
#define ATAPI_LOCATE			0x2B	/* locate to position */
#define ATAPI_READ_POSITION		0x34	/* read position */
#define ATAPI_SYNCHRONIZE_CACHE		0x35	/* flush buf, close channel */
#define ATAPI_WRITE_BUFFER		0x3B	/* write device buffer */
#define ATAPI_READ_BUFFER		0x3C	/* read device buffer */
#define ATAPI_READ_SUBCHANNEL		0x42	/* get subchannel info */
#define ATAPI_READ_TOC			0x43	/* get table of contents */
#define ATAPI_PLAY_10			0x45	/* play by lba */
#define ATAPI_PLAY_MSF			0x47	/* play by MSF address */
#define ATAPI_PLAY_TRACK		0x48	/* play by track number */
#define ATAPI_PAUSE			0x4B	/* pause audio operation */
#define ATAPI_READ_DISK_INFO		0x51	/* get disk info structure */
#define ATAPI_READ_TRACK_INFO		0x52	/* get track info structure */
#define ATAPI_RESERVE_TRACK		0x53	/* reserve track */
#define ATAPI_SEND_OPC_INFO		0x54	/* send OPC structurek */
#define ATAPI_MODE_SELECT_BIG		0x55	/* set device parameters */
#define ATAPI_REPAIR_TRACK		0x58	/* repair track */
#define ATAPI_READ_MASTER_CUE		0x59	/* read master CUE info */
#define ATAPI_MODE_SENSE_BIG		0x5A	/* get device parameters */
#define ATAPI_CLOSE_TRACK		0x5B	/* close track/session */
#define ATAPI_READ_BUFFER_CAPACITY	0x5C	/* get buffer capicity */
#define ATAPI_SEND_CUE_SHEET		0x5D	/* send CUE sheet */
#define ATAPI_SERVICE_ACTION_IN		0x96	/* get service data */
#define ATAPI_BLANK			0xA1	/* blank the media */
#define ATAPI_SEND_KEY			0xA3	/* send DVD key structure */
#define ATAPI_REPORT_KEY		0xA4	/* get DVD key structure */
#define ATAPI_PLAY_12			0xA5	/* play by lba */
#define ATAPI_LOAD_UNLOAD		0xA6	/* changer control command */
#define ATAPI_READ_STRUCTURE		0xAD	/* get DVD structure */
#define ATAPI_PLAY_CD			0xB4	/* universal play command */
#define ATAPI_SET_SPEED			0xBB	/* set drive speed */
#define ATAPI_MECH_STATUS		0xBD	/* get changer status */
#define ATAPI_READ_CD			0xBE	/* read data */
#define ATAPI_POLL_DSC			0xFF	/* poll DSC status bit */

void ata_firmware_init(void);

#endif /* __DRIVER_ATA_H__ */