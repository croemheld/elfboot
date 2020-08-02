#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/pci.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

LIST_HEAD(pci_devs);

#ifdef CONFIG_DEBUG

static const char *pci_dev_classes[] = {
	"Unclassified Device",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
	"Intelligent Controller",
	"Satellite Communication Controller",
	"Encryption Controller",
	"Signal Processing Controller",
	"Processing Accelerator",
	"Non-Essential Instrumentation"
};

static const char *pci_class_name(struct pci_dev *pcidev)
{
	return pci_dev_classes[pcidev->class];
}

#endif /* CONFIG_DEBUG */

static void pci_dump_device(struct pci_dev *pcidev)
{
#ifdef CONFIG_DEBUG
	const char *name = pci_class_name(pcidev);
#else
	const char *name = "";
#endif /* CONFIG_DEBUG */

	bprintln("PCI: %02x:%02x.%01x [%04lx] (%02x): %04lx:%04lx %s",
		 pcidev->addr.bus, pcidev->addr.slot, pcidev->addr.func,
		 pcidev->classrv >> 16, (pcidev->classrv >> 8) & 0xff,
		 pcidev->vendor, pcidev->device, name);
}

/*
 * Raw PCI reading and writing
 */

static uint8_t  read_pci_config_byte(uint8_t bus, uint8_t slot, uint8_t func,
				    uint8_t offset)
{
	uint8_t v;
	
	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	v = inb(PCI_CONFIG_DATA + (offset & 3));
	
	return v;
}

static uint16_t read_pci_config_word(uint8_t bus, uint8_t slot, uint8_t func,
				     uint8_t offset)
{
	uint16_t v;
	
	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	v = inw(PCI_CONFIG_DATA + (offset & 2));
	
	return v;
}

static uint32_t read_pci_config_long(uint8_t bus, uint8_t slot, uint8_t func,
				     uint8_t offset)
{
	uint32_t v;

	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	v = inl(PCI_CONFIG_DATA);
	
	return v;
}

static void write_pci_config_byte(uint8_t bus, uint8_t slot, uint8_t func,
				  uint8_t offset, uint8_t val)
{
	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	outb(val, PCI_CONFIG_DATA + (offset & 3));
}

static void write_pci_config_word(uint8_t bus, uint8_t slot, uint8_t func,
				  uint8_t offset, uint16_t val)
{
	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	outw(val, PCI_CONFIG_DATA + (offset & 2));
}

static void write_pci_config_long(uint8_t bus, uint8_t slot, uint8_t func,
				  uint8_t offset, uint32_t val)
{
	outl(PCI_CONFIG_ADDR, PCI_CONFIG_REGISTER(bus, slot, func, offset));
	outl(val, PCI_CONFIG_DATA);
}

/*
 * PCI device utility functions for reading and writing
 */

uint8_t  pci_read_config_byte(struct pci_address *addr, uint8_t offset)
{
	return read_pci_config_byte(addr->bus, addr->slot, addr->func, offset);
}

uint16_t pci_read_config_word(struct pci_address *addr, uint8_t offset)
{
	return read_pci_config_word(addr->bus, addr->slot, addr->func, offset);
}

uint32_t pci_read_config_long(struct pci_address *addr, uint8_t offset)
{
	return read_pci_config_long(addr->bus, addr->slot, addr->func, offset);
}

void pci_write_config_byte(struct pci_address *addr, uint8_t offset, uint8_t val)
{
	write_pci_config_byte(addr->bus, addr->slot, addr->func, offset, val);
}

void pci_write_config_word(struct pci_address *addr, uint8_t offset, uint16_t val)
{
	write_pci_config_word(addr->bus, addr->slot, addr->func, offset, val);
}

void pci_write_config_long(struct pci_address *addr, uint8_t offset, uint32_t val)
{
	write_pci_config_long(addr->bus, addr->slot, addr->func, offset, val);
}

/*
 * PCI utility functions
 */

static bool pci_probe_byte(struct pci_address *addr, uint8_t offset, uint8_t val)
{
	return pci_read_config_byte(addr, offset) == val;
}

static bool pci_probe_word(struct pci_address *addr, uint8_t offset, uint16_t val)
{
	return pci_read_config_word(addr, offset) == val;
}

static bool pci_probe_long(struct pci_address *addr, uint8_t offset, uint32_t val)
{
	return pci_read_config_long(addr, offset) == val;
}

/*
 * PCI Configuration Space
 */

static bool pci_probe_device(struct pci_address *addr)
{
	return pci_probe_long(addr, PCI_VENDOR, PCI_INVALID_DEVICE);
}

static bool pci_probe_class(struct pci_address *addr, uint8_t class)
{
	return pci_probe_byte(addr, PCI_CLASS, class);
}

static bool pci_probe_subclass(struct pci_address *addr, uint8_t subclass)
{
	return pci_probe_byte(addr, PCI_SUBCLASS, subclass);
}

static bool pci_probe_multifunction(struct pci_address *addr)
{
	return !!(pci_read_config_byte(addr, PCI_HEADER_TYPE) & 0x80);
}

/*
 * PCI device configuration
 */

void pci_set_master(struct pci_dev *pcidev)
{
	uint16_t command = pci_read_config_word(&pcidev->addr, PCI_COMMAND);

	if (command & PCI_COMMAND_BUSMASTER)
		return;

	command |= PCI_COMMAND_BUSMASTER;

	pci_write_config_word(&pcidev->addr, PCI_COMMAND, command);
}

/*
 * PCI devices
 */

static struct pci_dev *pci_get_subsys(struct pci_dev_id *id, struct pci_dev *from)
{
	struct pci_dev *pcidev;

	list_for_each_entry(pcidev, (from) ? &from->list : &pci_devs, list) {
		if ((id->vendor == PCI_ANY_ID || 
		     id->vendor == pcidev->vendor) &&
		    (id->device == PCI_ANY_ID || 
		     id->device == pcidev->device) &&
		    (id->subvendor == PCI_ANY_ID ||
		     id->subvendor == pcidev->subvendor) &&
		    (id->subdevice == PCI_ANY_ID ||
		     id->subdevice == pcidev->subdevice) &&
		    !((id->class ^ pcidev->classrv) & PCI_CLASS_MASK))
			return pcidev;
	}

	return NULL;
}

struct pci_dev *pci_get_device(uint16_t vendor, uint16_t device, struct pci_dev *from)
{
	struct pci_dev_id id = {
		.vendor = vendor,
		.device = device,
		.subvendor = PCI_ANY_ID,
		.subdevice = PCI_ANY_ID,
		.class = PCI_ANY_ID
	};

	return pci_get_subsys(&id, from);
}

struct pci_dev *pci_get_class(uint32_t class, struct pci_dev *from)
{
	struct pci_dev_id id = {
		.vendor = PCI_ANY_ID,
		.device = PCI_ANY_ID,
		.subvendor = PCI_ANY_ID,
		.subdevice = PCI_ANY_ID,
		.class = class
	};

	return pci_get_subsys(&id, from);
}

static struct pci_dev *pci_find_device_by_address(struct pci_address *addr)
{
	struct pci_dev *pcidev;

	list_for_each_entry(pcidev, &pci_devs, list) {

		/*
		 * Compare the entire PCI address to the one searched for.
		 * This approach is useful if no vendor or device is given
		 * for a PCI device lookup (e.g. EDD).
		 */

		if (!memcmp(addr, &pcidev->addr, sizeof(*addr)))
			return pcidev;
	}

	return NULL;
}

struct pci_dev *pci_find_device(uint16_t bus, uint16_t slot, uint16_t func)
{
	struct pci_address addr = {
		.bus  = bus,
		.slot = slot,
		.func = func
	};

	return pci_find_device_by_address(&addr);
}

/*
 * PCI initialization
 */

static void pci_probe_bus(struct pci_address *addr, struct pci_dev *parent);

static void pci_probe_bridge(struct pci_address *addr, struct pci_dev *parent)
{
	struct pci_address bridge = { 0 };

	if ((pci_probe_class(addr, PCI_CLASS_BRIDGE)) &&
	    (pci_probe_subclass(addr, PCI_SUBCLASS_BRIDGE_PCI))) {
		bridge.bus = pci_read_config_byte(addr, PCI_SECONDARY_BUS);

		/*
		 * Start scanning from the secondary bus on the other
		 * side of the PCI-to-PCI Bridge.
		 */

		pci_probe_bus(&bridge, parent);
	}
}

static void pci_alloc_device(struct pci_address *addr, struct pci_dev *parent)
{
	struct pci_dev *pcidev = bmalloc(sizeof(*pcidev));

	if (!pcidev)
		return;

	list_init(&pcidev->list);

	/* Base address of new device */
	pcidev->addr.bus  = addr->bus;
	pcidev->addr.slot = addr->slot;
	pcidev->addr.func = addr->func;

	/* Configuration Space of new device */
	pcidev->vendor  = pci_read_config_word(addr, PCI_VENDOR);
	pcidev->device  = pci_read_config_word(addr, PCI_DEVICE);
	pcidev->command = pci_read_config_word(addr, PCI_COMMAND);
	pcidev->status  = pci_read_config_word(addr, PCI_STATUS);
	pcidev->classrv = pci_read_config_long(addr, PCI_REVISION);
	pcidev->bar[0]  = pci_read_config_long(addr, PCI_BAR0);
	pcidev->bar[1]  = pci_read_config_long(addr, PCI_BAR1);
	pcidev->bar[2]  = pci_read_config_long(addr, PCI_BAR2);
	pcidev->bar[3]  = pci_read_config_long(addr, PCI_BAR3);
	pcidev->bar[4]  = pci_read_config_long(addr, PCI_BAR4);
	pcidev->bar[5]  = pci_read_config_long(addr, PCI_BAR5);
	pcidev->subvendor = pci_read_config_word(addr, PCI_SUBVENDOR);
	pcidev->subdevice = pci_read_config_word(addr, PCI_SUBDEVICE);

	pci_dump_device(pcidev);

	list_add(&pcidev->list, &pci_devs);

	/* Case: PCI-to-PCI Bridge */
	pci_probe_bridge(addr, pcidev);
}

static void pci_probe_func(struct pci_address *addr, struct pci_dev *parent)
{
	/* Case: No device */
	if (pci_probe_device(addr))
		return;

	/* We don't want duplicate entries for a device, skip */
	if (pci_find_device(addr->bus, addr->slot, addr->func))
		return;

	/*
	 * At this point, we made sure the device actually exists
	 * so we can use the address to create a new PCI device.
	 */

	pci_alloc_device(addr, parent);
}

static void pci_probe_slot(struct pci_address *addr, struct pci_dev *parent)
{
	/* Reset function */
	addr->func = 0;

	/* Case: No device */
	if (pci_probe_device(addr))
		return;

	/* Single device on func 0 */
	pci_probe_func(addr, parent);

	/* Skip non-multifunction devices */
	if (!pci_probe_multifunction(addr))
		return;

	for (addr->func = 1; addr->func < PCI_NUM_FUNCS; addr->func++)
		pci_probe_func(addr, parent);
}

static void pci_probe_bus(struct pci_address *addr, struct pci_dev *parent)
{
	for (addr->slot = 0; addr->slot < PCI_NUM_SLOTS; addr->slot++)
		pci_probe_slot(addr, parent);
}

int pci_init(void)
{
	struct pci_address addr = { 0 };

	/*
	 * Multiple PCI host controllers?
	 */

	if (!pci_probe_multifunction(&addr)) {

		/*
		 * There is only one PCI host controller, so we don't
		 * need to iterate over all possible buses.
		 */

		pci_probe_bus(&addr, NULL);
	} else
		for (; addr.bus < PCI_NUM_BUSES; addr.bus++)
			pci_probe_bus(&addr, NULL);

	return 0;
}