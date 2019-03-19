#include <boot/boot.h>

#define SMAP                                      0x534d4150

static const char *memory_types[] = {
	"Invalid entry",
	"Available memory",
	"Reserved memory",
	"ACPI reclaimable memory",
	"ACPI NVS memory",
	"Bad memory"
};

static void e820_print_entry(struct e820_entry *entry)
{
	/*
	 * In real mode, problems occur when working with 64-bit types like
	 * uint64_t. To circumvent this issue, we cast it down to uint32_t
	 * which also perfectly fits the address range for the physical
	 * address space.
	 */
	
	uint32_t addr, size, type;

	addr = entry->addr;
	size = entry->size;
	type = entry->type;

	bprintf("%08p | %08p | %s\n", addr, size, memory_types[type]);
}

void memory_dump(struct boot_params *boot_params)
{
	int i;

	bprintf("%10s | %10s | %s\n", "Address", "Size", "Type");

	for(i = 0; i < boot_params->e820_count; i++)
		e820_print_entry(&boot_params->e820_table[i]);
}

static void detect_memory_e820(struct boot_params *boot_params)
{
	struct biosregs ireg, oreg;
	struct e820_entry *desc;
	uint32_t count = 0;

	static struct e820_entry buf;

	desc = boot_params->e820_table;

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
	} while (ireg.ebx && count < ARRAY_SIZE(boot_params->e820_table));

	boot_params->e820_count = count;
}
 
void detect_memory(struct boot_params *boot_params)
{
	detect_memory_e820(boot_params);
}