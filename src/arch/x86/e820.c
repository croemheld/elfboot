#include <elfboot/core.h>
#include <elfboot/mm.h>
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

void e820_memory_dump(struct e820_table *table)
{
	int i;
	uint32_t addr, type, end;
	struct e820_entry *entry;

	bprintln("Number of e820 memory map entries: %u",  table->nr_entries);

	for(i = 0; i < table->nr_entries; i++) {
		entry = &table->entries[i];

		addr = entry->addr_32;
		end  = addr + entry->size_32 - 1;
		type = entry->type;

		bprintln("[%08p - %08p], %s", addr, end, memory_types[type]);
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

static void e820_memblock_setup(struct e820_table *table)
{
	int i;
	struct e820_entry *entry;

	for(i = 0; i < table->nr_entries; i++) {
		entry = &table->entries[i];

		if (entry->type != E820_MEMORY_TYPE_AVAILABLE)
			continue;

		memblock_add(entry->addr_32, entry->size_32);
	}
}

void detect_memory(struct boot_params *boot_params)
{
	uint16_t nr_entries = detect_memory_e820(&boot_params->e820_table);

	if (!nr_entries)
		return;

	boot_params->e820_table.nr_entries = nr_entries;

	e820_memory_dump(&boot_params->e820_table);

	e820_memblock_setup(&boot_params->e820_table);
}