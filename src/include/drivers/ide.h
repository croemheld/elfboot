#ifndef __DRIVER_IDE_H__
#define __DRIVER_IDE_H__

#define DRIVER_IDE			"IDE"

#define IDE_MAX_CHANNELS		2

#define IDE_PORT_PRIMARY		0x1f0
#define IDE_PORT_SECONDARY		0x170

#define IDE_CTRL_OFFSET			0x206

#define IS_IDE_PATA(clb, chb)		(clb == 0x00 && chb == 0x00)
#define IS_IDE_SATA(clb, chb)		(clb == 0x3c && chb == 0xc3)
#define IS_IDE_PATAPI(clb, chb)	(clb == 0x14 && chb == 0xeb)
#define IS_IDE_SATAPI(clb, chb)	(clb == 0x69 && chb == 0x96)

#define IS_IDE_FAULTY_ATA(clb, chb)	(clb == 0xf0 && chb == 0xff)

struct ide_dev {
	uint16_t io_base;
	uint16_t control;
	uint16_t channel;
	uint32_t bmaster;
	int slave;
	uint8_t disk;
	uint8_t irq;
	uint16_t *private;
};

#endif /* __DRIVER_IDE_H__ */