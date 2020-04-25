#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/memblock.h>

#include <asm/boot.h>
#include <asm/bios.h>

#include <uapi/asm/bootparam.h>

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
	uint32_t i, base, size;
	struct e820_entry *entry;

	for(i = 0; i < table->nr_entries; i++) {
		entry = &table->entries[i];

		if (entry->type != E820_MEMORY_TYPE_AVAILABLE)
			continue;

		/*
		 * For now, we want to limit the free memory regions to no override
		 * the kernel which is loaded at the earliest address of 1 MB. This
		 * check will allow us to limit the memory available to the specific
		 * limit represented by MEMBLOCK_LIMIT.
		 */

		if (entry->addr_32 + entry->size_32 <= MEMBLOCK_START)
			continue;

		if (entry->addr_32 >= MEMBLOCK_LIMIT)
			continue;

		base = max(entry->addr_32, MEMBLOCK_START);
		size = entry->addr_32 + entry->size_32 - base;

		/*
		 * Add chunk of free memory to memblock allocator.
		 */

		memblock_add(base, size);
	}
}

int detect_memory(struct boot_params *boot_params)
{
	uint16_t nr_entries = detect_memory_e820(&boot_params->e820_table);

	if (!nr_entries)
		return -ENOMEM;

	boot_params->e820_table.nr_entries = nr_entries;

	/*
	 * Initialize memblock allocator with the entries stores in the previously
	 * filled e820 memory map. We use the original entries from the memory map
	 * because the kernel will later simply override already filled regions.
	 */
	
	e820_memblock_setup(&boot_params->e820_table);

	return 0;
}