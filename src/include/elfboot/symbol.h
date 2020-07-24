#ifndef __ELFBOOT_SYMBOL_H__
#define __ELFBOOT_SYMBOL_H__

#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/list.h>

struct elfboot_symbol {
	char	*name;
	uint32_t addr;
	struct list_head list;
};

uint32_t symbol_lookup_name(const char *name);

const char *symbol_lookup(uint32_t addr);

const char *symbol_lookup_caller(uint32_t addr);

int symbol_parse_map(char *table);

#endif /* __ELFBOOT_SYMBOL_H__ */