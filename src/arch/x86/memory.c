#include <asm/boot.h>
#include "bda.h"
#include "regs.h"

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

static inline struct e820_entry *fetch_e820_entry(int index)
{
	if (index >= e820_count)
		return NULL;

	return &e820_table[index];
}

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

static struct e820_entry *bootmem_split_block(uint64_t addr, 
	uint64_t size, int eid)
{
	uint32_t msize;
	uint64_t pad_prev, pad_next;
	struct e820_entry *entry;

	entry = fetch_e820_entry(eid);

	if (!entry)
		return NULL;

	if (entry->addr <= addr) {

		/*
		 * Paddings between the defined block and the 
		 * current entry.
		 */

		pad_prev = addr - entry->addr;
		pad_next = entry->addr + entry->size - (addr + size);

		if (entry->addr + entry->size >= addr + size) {

			/*
			 * The defined block completely fits into the 
			 * current entry.
			 */

			if (!pad_prev && !pad_next)
				return entry;

			msize = (e820_count++ - eid) * sizeof(*entry);
			memmove(entry + 1, entry, msize);

			if (!pad_prev && pad_next) {
				entry->size = size;

				entry++;
				entry->addr += size;
				entry->size -= size;

				return --entry;
			}

			if (pad_prev && !pad_next) {
				entry->size -= size;

				entry++;
				entry->addr = addr;
				entry->size = size;

				return entry;
			}

			msize = (e820_count++ - eid) * sizeof(*entry);
			memmove(entry + 1, entry, msize);

			entry->size = pad_prev;

			entry++;
			entry->addr = addr;
			entry->size = size;

			entry++;
			entry->addr = addr + size;
			entry->size = pad_next;

			return --entry;
		} else {

			/*
			 * The defined block goes beyond the current entry.
			 */

			if (!pad_prev)
				return entry;

			msize = (e820_count++ - eid) * sizeof(*entry);
			memmove(entry + 1, entry, msize);

			entry->size = addr - entry->addr;

			entry++;
			entry->size = entry->addr + entry->size - addr;
			entry->addr = addr;

			return entry;
		}
	}

	return NULL;
}

static void bootmem_merge_blocks(void)
{
	int eid;
	uint32_t msize;
	uint64_t caddr, csize;
	struct e820_entry *centry, *nentry;

	for(eid = 0; eid < e820_count - 1; eid++) {
		centry = fetch_e820_entry(eid);
		nentry = fetch_e820_entry(eid + 1);

		if (!centry || !nentry)
			return;

		if (centry->addr + centry->size != nentry->addr)
			continue;

		if (centry->type != nentry->type)
			continue;

		caddr = centry->addr;
		csize = centry->size;
		msize = (--e820_count - eid) * sizeof(*centry);

		memmove(centry, nentry, msize);

		/* Add info of previous block */
		centry->addr  = caddr;
		centry->size += csize;
	}
}

static void bootmem_create_block(uint64_t addr, uint64_t size, uint32_t type)
{
	int eid;
	uint64_t rem_addr, rem_size;
	struct e820_entry *entry;
	uint32_t ctype = -1;

	rem_addr = addr;
	rem_size = size;

	for(eid = 0; eid < e820_count; eid++) {
		entry = fetch_e820_entry(eid);

		if (!entry)
			break;

		if (entry->addr + entry->size < rem_addr)
			continue;

		if (entry->addr > rem_addr) {
			rem_size -= min(entry->addr - rem_addr, rem_size);
			rem_addr  = entry->addr;
		}

		if (!rem_size) {
			entry->type = ctype;

			break;
		}

		if (ctype != (entry + 1)->type)
			ctype = (entry + 1)->type;

		entry = bootmem_split_block(rem_addr, rem_size, eid);
		entry->type = type;

		if (entry->addr + entry->size < rem_addr + rem_size) {

			/*
			 * Obtain the new block and mark it accordingly.
			 */

			rem_addr += entry->size;
			rem_size -= entry->size;
		}

		eid++;
	}

	bootmem_merge_blocks();
}

void bootmem_reserve(uint64_t addr, uint64_t size)
{
	bootmem_create_block(addr, size, E820_MEMORY_TYPE_RESERVED);
}

void bootmem_release(uint64_t addr, uint64_t size)
{
	bootmem_create_block(addr, size, E820_MEMORY_TYPE_AVAILABLE);
}

void bootmem_init(struct boot_params *boot_params)
{
	uint32_t e820_tsize;

	e820_count = detect_memory_e820(e820_table);
	e820_tsize = e820_count * sizeof(*e820_table);

	/*
	 * We provide a copy of the original e820 table for our kernel.
	 *
	 * This is neccessary because the kernel may reserve or use different
	 * memory regions than the bootloader. 
	 * 
	 * All reservations made to the e820 table in the bootloader do not 
	 * affect the e820 table for the kernel.
	 */

	boot_params->e820_count = e820_count;
	memcpy(boot_params->e820_table, e820_table, e820_tsize);

	/*
	 * Reservations for the bootmem allocator.
	 *
	 * All memory regions which should not be allocated should be 
	 * reserved in this function. Reservations after the memory 
	 * allocator has been set up may cause undefined behavior in 
	 * the bootloader.
	 */

	/* Interrupt Vector Table */
	bootmem_reserve(IVT_ADDRESS, IVT_MAX_SIZE);

	/* BIOS Data Area */
	bootmem_reserve(BDA_ADDRESS, BDA_MAX_SIZE);

	/* Bootloader */
	bootmem_reserve(IMG_ADDRESS, IMG_MAX_SIZE);

	/* Extended BIOS Data Area */
	bootmem_reserve(EBDA_ADDRESS, EBDA_MAX_SIZE);

	/* Initialize bootloader memory allocator */
	bmalloc_init(e820_table, e820_count);
}