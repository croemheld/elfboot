#include <elfboot/core.h>
#include <elfboot/printf.h>

#include <asm/bios.h>
#include <asm/e820.h>

#include <uapi/asm/bootparam.h>

static const char *memory_types[] = {
	[E820_MEMORY_TYPE_INVALID] = "Invalid entry",
	[E820_MEMORY_TYPE_AVAILABLE] = "Available memory",
	[E820_MEMORY_TYPE_RESERVED] = "Reserved memory",
	[E820_MEMORY_TYPE_ACPI_RECLAIMABLE] = "ACPI reclaimable memory",
	[E820_MEMORY_TYPE_ACPI_NVS] = "ACPI NVS memory",
	[E820_MEMORY_TYPE_BAD_MEMORY] = "Bad memory"
};

void e820_memory_dump(struct boot_params *boot_params)
{
	int i;
	uint32_t addr, size, type;
	struct e820_entry *entry;

	bprintf("%10s | %10s | %s\n", "Address", "Size", "Type");

	for(i = 0; i < boot_params->e820_table.nr_entries; i++) {
		entry = &boot_params->e820_table.entries[i];

		addr = entry->addr_32;
		size = entry->size_32;
		type = entry->type;

		bprintf("%08p | %08p | %s\n", addr, size, memory_types[type]);
	}
}

static uint16_t detect_memory_e820(struct e820_table *table)
{
	struct biosregs ireg, oreg;
	struct e820_entry *desc;
	uint32_t count = 0;

	static struct e820_entry buf;

	desc = table->entries;

	initregs(&ireg);
	ireg.ax  = 0xe820;
	ireg.cx  = sizeof(buf);
	ireg.edx = SMAP;
	ireg.di  = (size_t)&buf;

	do {
		bioscall(0x15, &ireg, &oreg);
		ireg.ebx = oreg.ebx;

		if (oreg.eflags & X86_EFLAGS_CF)
			break;

		if (oreg.eax != SMAP) {
			count = 0;
			break;
		}

		*desc++ = buf;
		count++;
	} while (ireg.ebx && count < E820_MAX_ENTRIES);

	return count;
}

static void e820_memblock_setup(void)
{

}

void detect_memory(struct boot_params *boot_params)
{
	uint16_t nr_entries = detect_memory_e820(&boot_params->e820_table);

	if (!nr_entries)
		return;

	boot_params->e820_table.nr_entries = nr_entries;

	e820_memory_dump(boot_params);

	e820_memblock_setup();
}