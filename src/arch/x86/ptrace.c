#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/symbol.h>
#include <elfboot/printf.h>

#include <asm/ptrace.h>
#include <asm/traps.h>

static const char *lookup_stack_symbol(uint32_t addr)
{
	const char *symbol = symbol_lookup_caller(addr);

	if (symbol)
		return symbol;

	return "<?>";
}

static void dump_stack_symbol(uint32_t addr)
{
	bprintln("PTRACE: %08lx (%s)", addr, lookup_stack_symbol(addr));
}

void __dump_stack(uint32_t eip, uint32_t ebp)
{
	int frame;

	dump_stack_symbol(eip);

	for (frame = 0; frame < PTRACE_MAX_FRAMES; frame++) {

		/*
		 * If we reach the last frame then the EBP should be zweo as we set
		 * it to that value when booting.
		 */
		if (!ebp)
			break;

		/*
		 * Get the return address to the previous stack frame. This address
		 * ca be in the middle of a function so it's most likely not usable
		 * for a symbol lookup.
		 */
		eip = ((uint32_t *)ebp)[1];

		/*
		 * We use the EBP as lookup value in order to search for the symbol
		 * it references. If there is no symbol it could mean either of the
		 * following things: We haven't yet loaded the symbol map or we see
		 * an address which does not belong to a symbol.
		 */
		dump_stack_symbol(eip);

		ebp = *((uint32_t *)ebp);
	}

	if (frame == PTRACE_MAX_FRAMES)
		bprintln("PTRACE: [...]");
}

void dump_stack(void)
{
	uint32_t eip, ebp;

	/*
	 * We use the address of the first instruction of this function for our
	 * stack dump. That means, the function dump_stack is also contained in
	 * the output.
	 */
	eip = tuint(dump_stack);
	asm volatile("movl %%ebp, %0" : "=r"(ebp));

	__dump_stack(eip, ebp);
}