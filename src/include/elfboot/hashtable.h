#ifndef __ELFBOOT_HASHTABLE_H__
#define __ELFBOOT_HASHTABLE_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/mm.h>
#include <elfboot/list.h>
#include <elfboot/math.h>

#include <uapi/elfboot/common.h>

#define GOLDEN_RATIO			0x61C88647

/*
 * Hashtable for all structures
 *
 * The bits member of the structure equals log2(size), i.e.
 * the size of the hashtable is always a power of 2.
 */

struct hlist_table {
	uint32_t bits;
	struct hlist_head *head;
};

#define DEFINE_HLIST_TABLE(___n, ___b)					\
	struct hlist_table ___n = {					\
		.bits = ___b,						\
		.head = NULL,						\
	};

static __always_inline size_t hashtable_size(struct hlist_table *table)
{
	return (1UL << table->bits);
}

static inline int hlist_table_init(struct hlist_table *hashtable)
{
	size_t size = (1UL << hashtable->bits);

	hashtable->head = bmalloc(size * sizeof(*(hashtable->head)));
	if (!hashtable->head)
		return -ENOMEM;

	return 0;
}

/*
 * Fibonacci hash function
 */

static inline uint32_t hash_32(uint32_t val, uint32_t bits)
{
	return (val * GOLDEN_RATIO) >> (32 - bits);
}

static inline uint32_t hash_ptr(void *ptr, uint32_t bits)
{
	return hash_32(vptrtuint(ptr), bits);
}

#define hashtable_add_hashed(table, node, bkt)				\
	hlist_add_head(node, &table[bkt])

static inline bool hash_hashed(struct hlist_node *node)
{
	return !hlist_unhashed(node);
}

static inline void hash_del(struct hlist_node *node)
{
	hlist_del_init(node);
}

/*
 * Iterate over the entire hashtable
 */

#define hashtable_for_each(pos, table, bkt)				\
	for (bkt = 0; bkt < hashtable_size(table); bkt++)		\
		hlist_for_each(pos, &table->head[bkt])

#define hashtable_for_each_safe(pos, n, table, bkt)			\
	for (bkt = 0; bkt < hashtable_size(table); bkt++)		\
		hlist_for_each_safe(pos, n, &table->head[bkt])

#define hashtable_for_each_entry(pos, table, bkt)			\
	for (bkt = 0; bkt < hashtable_size(table); bkt++)		\
		hlist_for_each_entry(pos, &table->head[bkt])

#define hashtable_for_each_entry_safe(pos, n, table, bkt)		\
	for (bkt = 0; bkt < hashtable_size(table); bkt++)		\
		hlist_for_each_entry_safe(pos, n, &table->head[bkt])

/*
 * Iterate over every entry inside a hashtable bucket
 */

#define hashtable_for_each_hash(pos, table, bkt)			\
	hlist_for_each(pos, &table->head[bkt])

#define hashtable_for_each_hash_safe(pos, n, table, bkt)		\
	hlist_for_each_safe(pos, n, &table->head[bkt])

#define hashtable_for_each_hash_entry(pos, table, bkt, member)		\
	hlist_for_each_entry(pos, &table->head[bkt], member)

#define hashtable_for_each_hash_entry_safe(pos, n, table, bkt, member)	\
	hlist_for_each_entry_safe(pos, n, &table->head[bkt], member)

#endif /* __ELFBOOT_HASHTABLE_H__ */