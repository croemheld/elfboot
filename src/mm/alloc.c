#include <elfboot/alloc.h>

LIST_HEAD(alloc_free);

static void bfree_merge_free_list(void)
{
	struct alloc_node *ablk, *lblk = NULL;
	void *aptr;

	list_for_each_entry(ablk, &alloc_free, node) {

		if (!lblk) {
			lblk = ablk;

			continue;
		}

		/*
		 * Merge two adjacent blocks to one large block.
		 *
		 * We simply check if the size of the block added to the address
		 * matches the address of the next block in the alloc_free list.
		 */
		
		aptr = ablk;
			
		if ((&lblk->data + lblk->size) == aptr) {
			lblk->size += sizeof(*ablk) + ablk->size;

			list_del(&ablk->node);
		}
	}
}

void *bmalloc(size_t size)
{
	struct alloc_node *ablk, *nblk;
	void *dptr;

	ablk = dptr = NULL;

	list_for_each_entry(ablk, &alloc_free, node) {

		/*
		 * First fit algorithm for boot memory allocation.
		 *
		 * At this point, performance it not really an issue which 
		 * is why we decide to use an extremely simple algorithm.
		 */

		if (ablk->size >= size) {
			dptr = &ablk->data;

			break;
		}
	}

	if (!dptr)
		return NULL;

	if ((ablk->size - size) >= ALLOC_NODE_MIN_SIZE) {
		nblk = dptr + size;

		nblk->size = ablk->size - size - ALLOC_NODE_HDR_SIZE;
		ablk->size = size;

		list_add(&nblk->node, &ablk->node);
	}

	list_del(&ablk->node);

	return dptr;
}

void *bzalloc(size_t size)
{
	void *dptr = bmalloc(size);

	if (!dptr)
		return NULL;

	memset(dptr, 0, size);

	return dptr;
}

char *bstrdup(const char *str)
{
	char *strdup;
	int strsize;

	strsize = strlen(str);
	strdup = bmalloc(strsize + 1);

	if (!strdup)
		return NULL;

	memcpy(strdup, str, strsize);
	strdup[strsize] = '\0';

	return strdup;
}

static void __bfree(void *dptr)
{
	struct alloc_node *ablk, *fblk;

	ablk = container_of(dptr, struct alloc_node, data);

	list_for_each_entry(fblk, &alloc_free, node) {

		/*
		 * Find the first free block after the pointer and insert the 
		 * corresponding node in the free list.
		 */
		
		if (fblk > ablk) {
			__list_add(&ablk->node, fblk->node.prev, &fblk->node);

			return;
		}
	}

	list_add_tail(&ablk->node, &alloc_free);
}

void bfree(void *dptr)
{
	if (!dptr)
		return;

	__bfree(dptr);

	/*
	 * Merge adjacent blocks to one large block to reduce fragmentation.
	 */

	bfree_merge_free_list();
}

void *brealloc(void *dptr, size_t size)
{
	void *nptr;
	struct alloc_node *ablk;

	if (!dptr)
		return bmalloc(size);

	if (!size) {
		bfree(dptr);

		return NULL;
	}

	nptr = bmalloc(size);

	if (!nptr)
		return NULL;

	ablk = container_of(dptr, struct alloc_node, data);

	if (ablk->size > size)
		memcpy(nptr, dptr, size);
	else
		memcpy(nptr, dptr, ablk->size);

	bfree(dptr);

	return nptr;
}

static void bmalloc_create_free_list(uint32_t addr, size_t size)
{
	struct alloc_node *ablk;

	/*
	 * Since the address is passed on from the memory map stored in the
	 * boot_params structure, it is guaranteed to be page-aligned.
	 */

	ablk = uinttvptr(addr);
	ablk->size = size - ALLOC_NODE_HDR_SIZE;

	list_add_tail(&ablk->node, &alloc_free);

	bprintf("Adding free memory list at %08p, size %08p bytes\n", 
		ablk, ablk->size);
}

void bmalloc_init(struct e820_entry *e820_table, uint16_t e820_count)
{
	struct e820_entry *entry;

	array_for_each_entry(entry, e820_table, e820_count) {

		/*
		 * The bootloader is running in 32-bit protected mode with full 
		 * access to 4 GiB of physical memory. As such we simply add all
		 * available memory regions to our memory allocator.
		 */
		
		if (entry->type == E820_MEMORY_TYPE_AVAILABLE)
			bmalloc_create_free_list(entry->addr, entry->size);
	}
}