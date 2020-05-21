#ifndef __ELFBOOT_PCI_H__
#define __ELFBOOT_PCI_H__

#include <elfboot/core.h>
#include <elfboot/list.h>

#include <uapi/elfboot/const.h>

/*
 * PCI device enumeration defines
 */

#define PCI_NUM_FUNCS		_BITUL(3)
#define PCI_NUM_SLOTS		_BITUL(5)
#define PCI_NUM_BUSES		_BITUL(8)

/*
 * PCI configuration ports
 */

#define PCI_CONFIG_ADDR		0xcf8
#define PCI_CONFIG_DATA		0xcfc

#define PCI_CONFIG_REGISTER(bus, slot, func, offset)	\
	((1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | offset)

/*
 * Fixed PCI fields
 */

#define PCI_VENDOR			0x00
#define PCI_DEVICE			0x02
#define PCI_COMMAND			0x04
#define PCI_STATUS			0x06
#define PCI_REVISION		0x08
#define PCI_PROG_IF			0x09
#define PCI_SUBCLASS		0x0A
#define PCI_CLASS			0x0B
#define PCI_CLS				0x0C
#define PCI_LATENCY			0x0D
#define PCI_HEADER_TYPE		0x0E
#define PCI_BIST			0x0F

#define PCI_INVALID_DEVICE	(~0UL)
#define PCI_ANY_ID			(~0UL)

#define PCI_COMMAND_BUSMASTER	_BITUL(2)

#define PCI_CLASS_MASK		(~(_BITUL(8) - 1))
	
#define PCI_DEVICE_CLASS(c, s, p) ((c << 24) | (s << 16) | (p << 8))

/*
 * PCI Header Type 0x00
 */

#define PCI_BAR0			0x10
#define PCI_BAR1			0x14
#define PCI_BAR2			0x18
#define PCI_BAR3			0x1C
#define PCI_BAR4			0x20
#define PCI_BAR5			0x24
#define PCI_CARDBUS			0x28
#define PCI_SUBVENDOR		0x2C
#define PCI_SUBDEVICE		0x2E
#define PCI_EROM			0x30
#define PCI_CAPABILITIES	0x34
#define PCI_INTR_LINE		0x3C
#define PCI_INTR_PIN		0x3D
#define PCI_MIN_GRANT		0x3E
#define PCI_MAX_LATENCY		0x3F

#define PCI_BAR_MASK		0xfffffffc

/*
 * PCI Header Type 0x00
 */

#define PCI_PRIMARY_BUS		0x18
#define PCI_SECONDARY_BUS	0x19

/*
 * PCI Classes and Subclasses
 */

#define PCI_CLASS_UNKNOWN				0x00
#define PCI_SUBCLASS_UNKNOWN_NONVGA		0x00
#define PCI_SUBCLASS_UNKNOWN_VGA		0x01

#define PCI_CLASS_STORAGE				0x01
#define PCI_SUBCLASS_STORAGE_SCSI		0x00
#define PCI_SUBCLASS_STORAGE_IDE		0x01
#define PCI_SUBCLASS_STORAGE_FLOPPY		0x02
#define PCI_SUBCLASS_STORAGE_IPI		0x03
#define PCI_SUBCLASS_STORAGE_RAID		0x04
#define PCI_SUBCLASS_STORAGE_ATA		0x05
#define PCI_SUBCLASS_STORAGE_SATA		0x06
#define PCI_SUBCLASS_STORAGE_SAS		0x07
#define PCI_SUBCLASS_STORAGE_MEM		0x08

#define PCI_CLASS_IDE 					\
	PCI_DEVICE_CLASS(PCI_CLASS_STORAGE, PCI_SUBCLASS_STORAGE_IDE, 0x80)

#define PCI_CLASS_NETWORK				0x02
#define PCI_SUBCLASS_NETWORK_ETH		0x00
#define PCI_SUBCLASS_NETWORK_TOKEN		0x01
#define PCI_SUBCLASS_NETWORK_FDDI		0x02
#define PCI_SUBCLASS_NETWORK_ATM		0x03
#define PCI_SUBCLASS_NETWORK_ISDN		0x04
#define PCI_SUBCLASS_NETWORK_WF			0x05
#define PCI_SUBCLASS_NETWORK_PICMG		0x06
#define PCI_SUBCLASS_NETWORK_INFI		0x07
#define PCI_SUBCLASS_NETWORK_FABRIC		0x08

#define PCI_CLASS_DISPLAY				0x03
#define PCI_SUBCLASS_DISPLAY_VGA		0x00
#define PCI_SUBCLASS_DISPLAY_XGA		0x01
#define PCI_SUBCLASS_DISPLAY_3D			0x02

#define PCI_CLASS_MEDIA					0x04
#define PCI_SUBCLASS_MEDIA_CON_VIDEO	0x00
#define PCI_SUBCLASS_MEDIA_CON_AUDIO	0x01
#define PCI_SUBCLASS_MEDIA_DEV_COM		0x02
#define PCI_SUBCLASS_MEDIA_DEV_AUDIO	0x03

#define PCI_CLASS_MEMORY				0x05
#define PCI_SUBCLASS_MEMORY_MEM			0x00
#define PCI_SUBCLASS_MEMORY_FLASH		0x01

#define PCI_CLASS_BRIDGE				0x06
#define PCI_SUBCLASS_BRIDGE_HOST		0x00
#define PCI_SUBCLASS_BRIDGE_ISA			0x01
#define PCI_SUBCLASS_BRIDGE_EISA		0x02
#define PCI_SUBCLASS_BRIDGE_MCA			0x03
#define PCI_SUBCLASS_BRIDGE_PCI			0x04
#define PCI_SUBCLASS_BRIDGE_PCMCIA		0x05
#define PCI_SUBCLASS_BRIDGE_NUBUS		0x06
#define PCI_SUBCLASS_BRIDGE_CARDBUS		0x07
#define PCI_SUBCLASS_BRIDGE_RACE		0x08
#define PCI_SUBCLASS_BRIDGE_PCI_SEMI	0x09
#define PCI_SUBCLASS_BRIDGE_INFI		0x0A

#define PCI_CLASS_COMM					0x07
#define PCI_SUBCLASS_COMM_SERIAL		0x00
#define PCI_SUBCLASS_COMM_PARALLEL		0x01
#define PCI_SUBCLASS_COMM_MULTIPORT		0x02
#define PCI_SUBCLASS_COMM_MODEM			0x03
#define PCI_SUBCLASS_COMM_IEEE			0x04
#define PCI_SUBCLASS_COMM_SMARTCARD		0x05

#define PCI_CLASS_PERIF					0x08
#define PCI_SUBCLASS_PERIF_PIC			0x00
#define PCI_SUBCLASS_PERIF_DMA			0x01
#define PCI_SUBCLASS_PERIF_TIMER		0x02
#define PCI_SUBCLASS_PERIF_RTC			0x03
#define PCI_SUBCLASS_PERIF_PCI			0x04
#define PCI_SUBCLASS_PERIF_SD			0x05
#define PCI_SUBCLASS_PERIF_IOMMU		0x06

#define PCI_CLASS_INPUT					0x09
#define PCI_SUBCLASS_INPUT_KEYBOARD		0x00
#define PCI_SUBCLASS_INPUT_PEN			0x01
#define PCI_SUBCLASS_INPUT_MOUSE		0x02
#define PCI_SUBCLASS_INPUT_SCANNER		0x03
#define PCI_SUBCLASS_INPUT_GAMEPORT		0x04

#define PCI_CLASS_DOCKING				0x0A
#define PCI_SUBCLASS_DOCKING_GENERIC	0x00

#define PCI_CLASS_PROC					0x0B
#define PCI_SUBCLASS_PROC_386			0x00
#define PCI_SUBCLASS_PROC_486			0x01
#define PCI_SUBCLASS_PROC_PENTIUM		0x02
#define PCI_SUBCLASS_PROC_PENTIUM_PRO	0x03
#define PCI_SUBCLASS_PROC_ALPHA			0x10
#define PCI_SUBCLASS_PROC_POWERPC		0x20
#define PCI_SUBCLASS_PROC_MIPS			0x30
#define PCI_SUBCLASS_PROC_COPROC		0x40

#define PCI_CLASS_SERIAL				0x0C
#define PCI_SUBCLASS_SERIAL_FIREWIRE	0x00
#define PCI_SUBCLASS_SERIAL_ACCESS		0x01
#define PCI_SUBCLASS_SERIAL_SSA			0x02
#define PCI_SUBCLASS_SERIAL_USB			0x03
#define PCI_SUBCLASS_SERIAL_FIBRE		0x04
#define PCI_SUBCLASS_SERIAL_SMBUS		0x05
#define PCI_SUBCLASS_SERIAL_INFI		0x06
#define PCI_SUBCLASS_SERIAL_IPME		0x07
#define PCI_SUBCLASS_SERIAL_SERCOS		0x08
#define PCI_SUBCLASS_SERIAL_CANBUS		0x09

#define PCI_CLASS_WIRELESS				0x0D
#define PCI_SUBCLASS_WIRELESS_IRDA		0x00
#define PCI_SUBCLASS_WIRELESS_IR		0x01
#define PCI_SUBCLASS_WIRELESS_RF		0x10
#define PCI_SUBCLASS_WIRELESS_BT		0x11
#define PCI_SUBCLASS_WIRELESS_BB		0x12
#define PCI_SUBCLASS_WIRELESS_ETH8021A	0x20
#define PCI_SUBCLASS_WIRELESS_ETH8021B	0x21

#define PCI_CLASS_INTELLIGENT			0x0E
#define PCI_SUBCLASS_INTELLIGENT_I20	0x00

#define PCI_CLASS_SATELLITE				0x0F
#define PCI_SUBCLASS_SATELLITE_TV		0x01
#define PCI_SUBCLASS_SATELLITE_AUDIO	0x02
#define PCI_SUBCLASS_SATELLITE_VOICE	0x03
#define PCI_SUBCLASS_SATELLITE_DATA		0x04

#define PCI_CLASS_ENC					0x10
#define PCI_SUBCLASS_ENC_NETCOM			0x00
#define PCI_SUBCLASS_ENC_ENTERTAINMENT	0x10

#define PCI_CLASS_SIGNAL				0x11
#define PCI_SUBCLASS_SIGNAL_DPIO		0x00
#define PCI_SUBCLASS_SIGNAL_PERF		0x01
#define PCI_SUBCLASS_SIGNAL_COMM		0x10
#define PCI_SUBCLASS_SIGNAL_MGMT		0x20

#define PCI_CLASS_ACCELERATOR			0x12

#define PCI_CLASS_INSTRUMENTATION		0x13

#define PCI_CLASS_COPROCESSOR			0x40

#define PCI_SUBCLASS_OTHER				0x80

/*
 * PCI device structures
 */

struct pci_address {

	/*
	 * Peripheral information
	 */

	uint16_t bus;
	uint16_t slot;
	uint16_t func;
};

struct pci_dev_id {
	uint32_t vendor;
	uint32_t device;
	uint32_t subvendor;
	uint32_t subdevice;

	/*
	 * Class, Subclass and Program Interface
	 */

	uint32_t class;
};

struct pci_dev {
	struct pci_address addr;

	/*
	 * PCI device structure according to the official PCI Specification
	 * (PCI-SIG).
	 */

	uint16_t vendor;
	uint16_t device;
	uint16_t command;
	uint16_t status;

	union {
		uint32_t classrv;
		struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			uint8_t revision;
			uint8_t prog_if;
			uint8_t subclass;
			uint8_t class;
#else
			uint8_t class;
			uint8_t subclass;
			uint8_t prog_if;
			uint8_t revision;
#endif
		};
	};

	/*
	 * Since we only register PCI devices whose header type equals 0x00.
	 * (i.e. endpoints), we always have all base address from BAR0 until
	 * BAR5 available for usage.
	 */
	uint32_t bar[6];

	uint32_t subvendor;
	uint32_t subdevice;

	/*
	 * List of registered PCI devices
	 */

	struct list_head list;
};

/*
 * PCI device utility functions for reading and writing
 */

uint8_t  pci_read_config_byte(struct pci_address *addr, uint8_t offset);

uint16_t pci_read_config_word(struct pci_address *addr, uint8_t offset);

uint32_t pci_read_config_long(struct pci_address *addr, uint8_t offset);

void pci_write_config_byte(struct pci_address *addr, uint8_t offset, uint8_t val);

void pci_write_config_word(struct pci_address *addr, uint8_t offset, uint16_t val);

void pci_write_config_long(struct pci_address *addr, uint8_t offset, uint32_t val);

void pci_set_master(struct pci_dev *pcidev);

struct pci_dev *pci_get_device(uint16_t vendor, uint16_t device, struct pci_dev *prev);

struct pci_dev *pci_get_class(uint32_t class, struct pci_dev *prev);

struct pci_dev *pci_find_device(uint16_t bus, uint16_t slot, uint16_t func);

int pci_init(void);

#endif /* __ELFBOOT_PCI_H__ */