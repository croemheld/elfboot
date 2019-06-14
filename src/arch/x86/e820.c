#include <elfboot/core.h>

uint16_t e820_count;
struct e820_entry e820_table[E820_MAX_ENTRIES];

static const char *memory_types[] = {
	"Invalid entry",
	"Available memory",
	"Reserved memory",
	"ACPI reclaimable memory",
	"ACPI NVS memory",
	"Bad memory"
};

void memory_dump(void)
{
	int i;
	uint32_t addr, size, type;
	struct e820_entry *entry;

	bprintf("%10s | %10s | %s\n", "Address", "Size", "Type");

	for(i = 0; i < e820_count; i++) {
		entry = fetch_e820_entry(i);

		addr = entry->addr;
		size = entry->size;
		type = entry->type;

		bprintf("%08p | %08p | %s\n", addr, size, memory_types[type]);
	}
}

static uint16_t detect_memory_e820(struct e820_entry *table)
{
	struct biosregs ireg, oreg;
	struct e820_entry *desc;
	uint32_t count = 0;

	static struct e820_entry buf;

	desc = table;

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
	uint16_t nr_entries = detect_memory_e820(&boot_params.e820_table);

	if (!nr_entries)
		return;

	boot_params->e820_table.nr_entries = nr_entries;

	e820_memblock_setup();
}