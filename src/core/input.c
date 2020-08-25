#include <elfboot/core.h>
#include <elfboot/linkage.h>
#include <elfboot/input.h>
#include <elfboot/interrupts.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

char *keybuffer = NULL;
static int keybuffer_index = 0;

/*
 * Keyboard functions
 */

static bool input_verify(char c)
{
	bool is_backspace = (c == KEY_BACKSPACE);

	if ((keybuffer_index >= KEYBOARD_BUFFER_SIZE) && !is_backspace)
		return false;

	if ((keybuffer_index <= 0) && is_backspace)
		return false;

	return true;
}

int input_copy_to_buffer(char c)
{
	bool is_backspace = (c == KEY_BACKSPACE);

	if (!input_verify(c))
		return -EFAULT;

	if (is_backspace)
		keybuffer[--keybuffer_index] = 0;
	else
		keybuffer[keybuffer_index++] = c;

	return 0;
}

static char __bgetch(void)
{
	char c = keybuffer[0];

	memcpy(keybuffer, keybuffer + 1, keybuffer_index--);

	return c;
}

unsigned char bgetch_noblock(void)
{
	if (!keybuffer_index)
		return 0;

	return __bgetch();
}

unsigned char bgetch(void)
{
	while (!keybuffer_index)
		arch_suspend_machine();

	return __bgetch();
}

int bgets(char *buffer, int length)
{
	int c, curpos;

	for (curpos = 0; curpos < length; curpos++) {
		c = bgetch();
		if (c == KEY_ENTER)
			return curpos + 1;

		buffer[curpos] = c;
	}

	return length;
}