#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/slub.h>
#include <elfboot/printf.h>
#include <elfboot/string.h>
#include <elfboot/math.h>
#include <elfboot/list.h>

static struct bmem_info bmalloc_info[] = {
	{"bmalloc-8", 		0x00000008},
	{"bmalloc-16", 		0x00000010},
	{"bmalloc-32", 		0x00000020},
	{"bmalloc-64", 		0x00000040},
	{"bmalloc-128", 	0x00000080},
	{"bmalloc-256", 	0x00000100},
	{"bmalloc-512", 	0x00000200},
	{"bmalloc-1024", 	0x00000400},
	{"bmalloc-2048", 	0x00000800},
	{"bmalloc-4096", 	0x00001000},
	{"bmalloc-8192", 	0x00002000},
	{"bmalloc-16384", 	0x00004000},
	{"bmalloc-32768", 	0x00008000},
};

static void bmem_cache_init(void *objp)
{
	struct bmem_cache *cachep = objp;

	/*
	 * Use this constructor to initialize all caches we
	 * need to set up all bmalloc caches for our system.
	 */

	cachep->free_objs   = 0;
	cachep->free_slabs  = 0;
	cachep->total_slabs = 0;
	list_init(&cachep->slabs_free);
	list_init(&cachep->slabs_partial);
	list_init(&cachep->slabs_full);
	cachep->ctor = NULL;
	list_init(&cachep->next);
}

/*
 * The cache for creating all other caches
 */
static struct bmem_cache bmem_cache = {
	.name = "bmem_cache",
	.ctor = bmem_cache_init,
};

/*
 * The actual caches for bmalloc()
 */
static struct bmem_cache *bmalloc_caches[KMALLOC_CACHE_NUM];

static LIST_HEAD(bmem_caches);

static inline int bmalloc_index(uint32_t size)
{
	if (!size)
		return 0;

	size = round_up_pow2(size);

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_MIN_SHIFT;

	if (size > KMALLOC_MIN_SIZE && size <= (1UL << 15))
		return log2(size) - 3;

	return -1;
}

static struct bmem_cache *bmalloc_cache(uint32_t size)
{
	int index = bmalloc_index(size);

	if (index < 0)
		return NULL;

	return bmalloc_caches[index];
}

static inline uint32_t calculate_cache_order(uint32_t size)
{
	uint32_t ret = log2(size - 1) + 1;

	return ret - min(ret, PAGE_SHIFT);
}

static void *bmem_cache_fetch(struct page *page)
{
	void *objp = page->freelist.next;

	if (objp == &page->freelist)
		return NULL;

	list_del(objp);
	page->inuse++;

	return objp;
}

static struct page *bmem_cache_page_alloc(struct bmem_cache *cachep)
{
	return alloc_pages(cachep->order);
}

static struct page *bmem_cache_page(struct bmem_cache *cachep)
{
	struct page *page;

	page = list_first_entry_or_null(&cachep->slabs_partial, struct page, list);
	if (page)
		return page;

	page = list_first_entry_or_null(&cachep->slabs_free, struct page, list);
	if (page)
		cachep->free_slabs--;

	return page;
}

static void bmem_cache_fixup(struct bmem_cache *cachep, struct page *page)
{
	list_del(&page->list);

	if (page->inuse == cachep->nums)
		list_move(&page->list, &cachep->slabs_full);
	else
		list_move(&page->list, &cachep->slabs_partial);
}

static void bmem_cache_setup(struct bmem_cache *cachep, struct page *page)
{
	struct list_head *objp;
	uint32_t objc;

	for (objc = 0; objc < cachep->nums; objc++) {
		objp = uinttvptr(page->paddr + cachep->size * objc);

		/* Initialize header */
		list_init(objp);

		/* Append to freelist */
		list_add_tail(objp, &page->freelist);

		/* Initialize object */
		if (cachep->ctor)
			cachep->ctor(objp);
	}
}

static struct page *bmem_cache_grow(struct bmem_cache *cachep)
{
	struct page *page = bmem_cache_page_alloc(cachep);

	if (!page)
		return NULL;

	/* Set the cache for freeing */
	page->slab_cache = cachep;

	/* Initialize objects */
	bmem_cache_setup(cachep, page);
	cachep->total_slabs++;

	if(!page->inuse) {
		list_move(&page->list, &cachep->slabs_free);
	} else
		bmem_cache_fixup(cachep, page);

	cachep->free_objs += cachep->nums - page->inuse;

	return page;
}

static struct page *__bmem_obj_page(void *objp)
{
	uint32_t addr = vptrtuint(objp);

	return phys_to_page(addr);
}

static struct page *bmem_obj_page(void *objp)
{
	struct page *page = __bmem_obj_page(objp);

	return compound_head(page);
}

static void __bfree(struct page *page, struct bmem_cache *cachep, void *objp)
{
	/* We treat the object as a list */
	struct list_head *new = objp;

	list_init(new);

	/* 
	 * Add the object back to the freelist. We don't really
	 * care about at which position the object was inserted
	 * so we simply add it at the end of the freelist.
	 */
	
	list_add_tail(new, &page->freelist);

	if (!--page->inuse) {
		list_move(&page->list, &cachep->slabs_free);
		cachep->free_slabs++;

		/*
		 * TODO CRO: Free free slabs? Limit of free slabs?
		 */
	} else {

		/*
		 * Before freeing the object, the slab is either
		 * full or partially allocated. Either way, it's
		 * neccessary to move the slab to the partially
		 * filled slab list.
		 */

		list_move(&page->list, &cachep->slabs_partial);
	}
}

void bfree(void *objp)
{
	struct page *page = bmem_obj_page(objp);

	__bfree(page, page->slab_cache, objp);
}

void *bmem_cache_alloc(struct bmem_cache *cachep)
{
	struct page *page;
	void *objp;

	page = bmem_cache_page(cachep);

	if (!page || !cachep->free_objs)
		page = bmem_cache_grow(cachep);

	objp = bmem_cache_fetch(page);
	if (!objp)
		return NULL;

	cachep->free_objs--;
	bmem_cache_fixup(cachep, page);

	return objp;
}

void *bmalloc(uint32_t size)
{
	struct bmem_cache *cachep = bmalloc_cache(size);

	if (!cachep)
		return NULL;

	return bmem_cache_alloc(cachep);
}

void *bzalloc(uint32_t size)
{
	void *objp = bmalloc(size);

	if (objp)
		memset(objp, 0, size);

	return objp;
}

struct bmem_cache *bmem_cache_create(const char *name, uint32_t size,
				     void (*ctor)(void *))
{
	struct bmem_cache *cachep = bmem_cache_alloc(&bmem_cache);

	if (!cachep)
		return NULL;

	cachep->name = bstrdup(name);
	if (!cachep->name)
		goto bmem_cache_alloc_free;

	cachep->size  = round_up_pow2(size);
	cachep->nums  = PAGE_SIZE / cachep->size;
	cachep->order = calculate_cache_order(cachep->size);

	cachep->ctor  = ctor;

	list_add(&cachep->next, &bmem_caches);

	return cachep;

bmem_cache_alloc_free:
	bfree(cachep);

	return NULL;
}

/*
 * Initialization
 */

static struct bmem_cache *bmalloc_create_cache(int cache_num)
{
	struct bmem_cache *cachep = bmem_cache_alloc(&bmem_cache);

	if (!cachep)
		return NULL;

	cachep->name  = bmalloc_info[cache_num].name;
	cachep->size  = bmalloc_info[cache_num].size;
	cachep->nums  = PAGE_SIZE / cachep->size;
	cachep->order = calculate_cache_order(cachep->size);

	return cachep;
}

static int bmalloc_create_all_caches(void)
{
	int cache_num;

	for (cache_num = 0; cache_num < KMALLOC_CACHE_NUM; cache_num++) {

		/*
		 * Allocate cache for all object sizes defined in the
		 * bmalloc_info structure array. If even one cache is
		 * failing during the setup, we cannot allocate memory
		 * for smaller objects at all.
		 */

		bmalloc_caches[cache_num] = bmalloc_create_cache(cache_num);

		if (!bmalloc_caches[cache_num])
			return -EFAULT;

		list_add(&bmalloc_caches[cache_num]->next, &bmem_caches);
	}

	return 0;
}

static void bmalloc_create_boot_cache(void)
{
	bmem_cache.size  = round_up_pow2(sizeof(struct bmem_cache));
	bmem_cache.nums  = PAGE_SIZE / bmem_cache.size;
	bmem_cache.order = calculate_cache_order(bmem_cache.size);
	bmem_cache.free_objs   = 0;
	bmem_cache.free_slabs  = 0;
	bmem_cache.total_slabs = 0;
	list_init(&bmem_cache.slabs_free);
	list_init(&bmem_cache.slabs_partial);
	list_init(&bmem_cache.slabs_full);
	list_init(&bmem_cache.next);

	/*
	 * Add root cache to list
	 */

	list_add(&bmem_cache.next, &bmem_caches);
}

int bmalloc_init(void)
{
	/* 
	 * Create boot cache for allocating the other
	 * bmalloc caches. That way we don't need to
	 * use an array for each bmem_cache we create.
	 */
	
	bmalloc_create_boot_cache();

	/*
	 * Allocate all other caches via the initial
	 * boot bmem_cache we set up beforehand.
	 */

	if (bmalloc_create_all_caches())
		return -EFAULT;

	return 0;
}