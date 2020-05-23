#include <elfboot/core.h>
#include <elfboot/mm.h>
#include <elfboot/symbol.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

LIST_HEAD(symbols);

static uint32_t num_symbols = 0;

static int symbol_add(char *name, uint32_t addr)
{
	struct elfboot_symbol *symbol = bmalloc(sizeof(*symbol));

	if (!symbol)
		return -ENOMEM;

	symbol->name = bstrdup(name);
	if (!symbol->name) {
		bfree(symbol);
		return -ENOMEM;
	}

	symbol->addr = addr;
	list_add_tail(&symbol->list, &symbols);

	num_symbols++;

	return 0;
}

uint32_t symbol_lookup_name(const char *name)
{
	struct elfboot_symbol *symbol;

	list_for_each_entry(symbol, &symbols, list) {
		if (strcmp(name, symbol->name))
			continue;

		return symbol->addr;
	}

	/*
	 * TODO CRO: Do we need to expand the search for modules too?
	 *
	 * If so, call function in core/module.c to iterate over all inserted
	 * modules and over all their symbols in the module structure.
	 */

	/*
	 * We consider a null value as an invalid symbol address, because the
	 * address points to the first page which is reserved for most systems
	 * anyway (IVT, ...).
	 */
	return 0;
}

const char *symbol_lookup(uint32_t addr)
{
	struct elfboot_symbol *symbol;

	list_for_each_entry(symbol, &symbols, list) {
		if (addr != symbol->addr)
			continue;

		return symbol->name;
	}

	/*
	 * TODO CRO: Do we need to expand the search for modules too?
	 *
	 * If so, call function in core/module.c to iterate over all inserted
	 * modules and over all their symbols in the module structure.
	 */

	return NULL;
}

const char *symbol_lookup_caller(uint32_t addr)
{
	struct elfboot_symbol *symbol, *caller = NULL;

	list_for_each_entry(symbol, &symbols, list) {
		if (symbol->addr >= addr)
			break;

		caller = symbol;
	}

	return caller->name;
}

int symbol_map_parse(char *syms)
{
	char *addr, *type, *name;
	const char delimiter[] = " \n";

	addr = strtok(syms, delimiter);

	while (addr) {
		type = strtok(NULL, delimiter);
		name = strtok(NULL, delimiter);

		/*
		 * To reduce the symbol map to be imported, we discard any symbol
		 * which starts with an underscore because, they usually refer to
		 * internal variables.
		 */
		if (name[0] == '_')
			goto parse_next_symbol;

		/*
		 * Since we created the symbol map using the "nm" command we know
		 * the layout of the resulting file on the boot device. Addresses
		 * are in hexadecimal format, but since it does not contain a "0x"
		 * prefix we cannot let it guess the base, which is why we need to
		 * tell it explicitely.
		 */
		if (symbol_add(name, strtoul(addr, NULL, 16)))
			return -ENOMEM;

parse_next_symbol:
		addr = strtok(NULL, delimiter);
	}

	bprintln("SYM: Imported %lu symbols from symbol map", num_symbols);

	return 0;
}