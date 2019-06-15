#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/sections.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <asm/boot.h>
#include <asm/bda.h>
#include <asm/video.h>

#include <uapi/asm/bootparam.h>

/*
 * Architecture-specific init function
 */

static struct boot_params boot_params;

static void bootmem_reserve_regions(void)
{
	/* Reserve memory for the Interrupt Vector Table */
	memblock_reserve(IVT_ADDRESS, IVT_MAX_SIZE);

	/* Reserve memory for the BIOS Data Area */
	memblock_reserve(BDA_ADDRESS, BDA_MAX_SIZE);

	/* Reserve memory for the Extended BIOS Data Area */
	memblock_reserve(EBDA_ADDRESS, EBDA_MAX_SIZE);

	/* Reserve memory for the entire bootloader */
	memblock_reserve(SECTION_START(boot), SECTION_SIZE(boot));
}

void main(uint8_t disk_drive)
{
	boot_params.disk_drive = disk_drive;

	/* Detect and set video modes */
	detect_videos(&boot_params);

	bprintln("fksölagheqohrnfkjqögklejrgöveqöjfrijgäirhqlhgjrnbvqrwjöklfrhqwkljfgklqwjfölkejröelwqkjröwlqkejröwelqkjrlökwqejrk");

	asm volatile("xchg %bx, %bx");

	bprintf("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n");

	/* Get memory map */
	detect_memory(&boot_params);

	/* 
	 * Right after setting up our memblock allocator, we need 
	 * to reserve all regions which should not be allocated 
	 * in order to prevent overriding the bootloader and other 
	 * important structures in memory.
	 */
	bootmem_reserve_regions();

	/* Dump memory map */
	memblock_dump();

	/* SLOB memory allocator */
	bmalloc_init();

	asm volatile("xchg %bx, %bx");

	detect_videos_vesa();
}