#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/memblock.h>
#include <elfboot/printf.h>

#include <asm/boot.h>
#include <asm/bios.h>

#include <uapi/asm/bootparam.h>

static uint16_t detect_memory_e820(struct e820_table *table)
{
	struct biosregs ireg, oreg;
	struct e820_entry *desc, buf;
	uint32_t count = 0;

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

static uint32_t detect_memory_lower(struct e820_table *table)
{
	uint32_t i;

	for(i = 0; i < table->nr_entries; i++) {
		if (table->entries[i].addr_32 == 0)
			return min(table->entries[i].size_32, 0xa0000);
	}

	return 0;
}

static uint32_t detect_memory_upper(struct e820_table *table)
{
	uint32_t i, base, size, upper = 0;

	for(i = 0; i < table->nr_entries; i++) {
		base = table->entries[i].addr_32;
		size = table->entries[i].size_32;

		if (base <= MEMORY_LIMIT && base + size > MEMORY_LIMIT) {
			upper = base + size - MEMORY_LIMIT;
		}
	}

	return upper;
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

		base = max(entry->addr_32, MEMBLOCK_START);
		size = entry->addr_32 + entry->size_32 - base;

		/*
		 * Add chunk of free memory to memblock allocator.
		 */

		if (entry->addr_32 >= MEMBLOCK_LIMIT)
			memblock_add_kernel(base, size);
		else
			memblock_add(base, size);
	}
}

int detect_memory(struct boot_params *boot_params)
{
	struct e820_table *table = &boot_params->e820_table;
	uint16_t nr_entries = detect_memory_e820(table);

	if (!nr_entries)
		return -ENOMEM;

	table->nr_entries = nr_entries;

	boot_params->memory_lower = detect_memory_lower(table) / 1024;
	boot_params->memory_upper = detect_memory_upper(table) / 1024;

	/*
	 * Initialize memblock allocator with the entries stores in the previously
	 * filled e820 memory map. We use the original entries from the memory map
	 * because the kernel will later simply override already filled regions.
	 */
	
	e820_memblock_setup(table);

	return 0;
}