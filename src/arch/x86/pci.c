#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/io.h>
#include <elfboot/pci.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>
#include <elfboot/list.h>

LIST_HEAD(pci_devices);

static const char *pci_device_classes[] = {
	"Unclassified Device",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller"
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

static const char *pci_class_name(struct pci_device *pcidev)
{
	return pci_device_classes[pcidev->class];
}

static void pci_dump_device(struct pci_device *pcidev)
{
	const char *name = pci_class_name(pcidev);

	bprintln("PCI: %s at %02u:%02u.%01u",
		 name, pcidev->addr.bus, pcidev->addr.slot, pcidev->addr.func);
	bprintln("PCI: Vendor %x, Device %x",
		 pcidev->vendor, pcidev->device);
	bprintln("PCI: Command %x, Status %x",
		 pcidev->command, pcidev->status);
	bprintln("PCI: Revision %x, PROG IF %x, Subclass %x, Class %x",
		 pcidev->revision, pcidev->prog_if, pcidev->subclass, pcidev->class);
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
 * PCI devices
 */

void pci_device_iterate(int (*hook)(struct pci_device *, void *), void *data)
{
	struct pci_device *pcidev;

	list_for_each_entry(pcidev, &pci_devices, list) {

		/*
		 * The hook function has to determine whether a PCI device
		 * fits the criteria for a specific device type.
		 *
		 * We only return from this function when critical errors
		 * occur during the iteration. Otherwise it just means that
		 * the specific device is not suitable for the driver.
		 */

		if (hook(pcidev, data))
			return;
	}
}

struct pci_device *pci_get_device(uint16_t vendor, uint16_t device)
{
	struct pci_device *pcidev;

	list_for_each_entry(pcidev, &pci_devices, list) {
		if ((pcidev->vendor == vendor) &&
		    (pcidev->device == device))
			return pcidev;
	}

	return NULL;
}

struct pci_device *pci_get_device_by_class(uint16_t class, uint8_t prog_if)
{
	struct pci_device *pcidev;

	list_for_each_entry(pcidev, &pci_devices, list) {
		if ((pcidev->prog_if  == prog_if) &&
		    (pcidev->subclass == ((class >> 0) & 0xff)) &&
		    (pcidev->class    == ((class >> 8) & 0xff)))
			return pcidev;
	}

	return NULL;	
}

/*
 * PCI initialization
 */

static void pci_alloc_device(struct pci_address *addr)
{
	struct pci_device *pcidev = bmalloc(sizeof(*pcidev));

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

	pci_dump_device(pcidev);

	list_add(&pcidev->list, &pci_devices);
}

static void pci_probe_bus(struct pci_address *addr);

static void pci_probe_bridge(struct pci_address *addr)
{
	struct pci_address bridge = { 0 };

	if ((pci_probe_class(addr, PCI_CLASS_BRIDGE)) &&
	    (pci_probe_subclass(addr, PCI_SUBCLASS_BRIDGE_PCI))) {
		bridge.bus = pci_read_config_byte(addr, PCI_SECONDARY_BUS);

		/*
		 * Start scanning from the secondary bus on the other
		 * side of the PCI-to-PCI Bridge.
		 */

		pci_probe_bus(&bridge);
	}
}

static void pci_probe_func(struct pci_address *addr)
{
	/* Case: No device */
	if (pci_probe_device(addr))
		return;

	/* Case: PCI-to-PCI Bridge */
	pci_probe_bridge(addr);

	/*
	 * At this point, we made sure the device actually exists
	 * so we can use the address to create a new PCI device.
	 */

	pci_alloc_device(addr);
}

static void pci_probe_slot(struct pci_address *addr)
{
	/* Case: No device */
	if (pci_probe_device(addr))
		return;

	/* Skip non-multifunction devices */
	if (!pci_probe_multifunction(addr))
		return;

	for (addr->func = 1; addr->func < PCI_NUM_FUNCS; addr->func++)
		pci_probe_func(addr);
}

static void pci_probe_bus(struct pci_address *addr)
{
	for (; addr->slot < PCI_NUM_SLOTS; addr->slot++)
		pci_probe_slot(addr);
}

void pci_init(void)
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

		pci_probe_bus(&addr);
	} else
		for (; addr.bus < PCI_NUM_BUSES; addr.bus++)
			pci_probe_bus(&addr);
}