#include <boot/boot.h>

char value[128];

static char *skip(char *entry, char skipping)
{
	int i;

	for(i = 0; entry[i] == skipping; i++);

	return &entry[i];
}

static char *__cmdline_get_value(char *entry)
{
	char *evalueptr, *delimiter, valuechar;
	int evaluelen = 0;

	delimiter = strchr(entry, '=');

	if(!delimiter)
		return NULL;

	evalueptr = skip(++delimiter, ' ');

	while(evaluelen < ARRAY_SIZE(value)) {
		valuechar = evalueptr[evaluelen];

		if((valuechar == ' ') || (valuechar == '\n'))
			break;

		evaluelen++;
	}

	memcpy(value, evalueptr, evaluelen);
	value[evaluelen] = 0;

	return value;
}

static char *cmdline_get_value(const char *key)
{
	size_t keylen;
	char *cmdline;

	keylen  = strlen(key);
	cmdline = uinttvptr(CMDLINE_BUFFER_ADDRESS);

	for(int i = 0; cmdline[i]; i++) {
		if(!strncmp(&cmdline[i], key, keylen))
			return __cmdline_get_value(&cmdline[i]);
	}

	return NULL;
}

bool cmdline_get_boolean_value(const char *key)
{
	char *res = cmdline_get_value(key);

	return !strncmp(res, "true", strlen(res));
}

bool cmdline_get_boolean_default_value(const char *key, bool default_value)
{
	char *res = cmdline_get_value(key);

	if(!res)
		return default_value;

	return !strncmp(res, "true", strlen(res));
}

uint32_t cmdline_get_int_value(const char *key)
{
	char *res = cmdline_get_value(key);

	/* Guess the base */
	return simple_strtol(res, NULL, 0);
}

uint32_t cmdline_get_int_default_value(const char *key, uint32_t default_value)
{
	char *res = cmdline_get_value(key);

	if(!res)
		return default_value;

	/* Guess the base */
	return simple_strtol(res, NULL, 0);
}

char *cmdline_get_string_value(const char *key)
{
	return cmdline_get_value(key);
}

char *cmdline_get_string_default_value(const char *key, const char *default_value)
{
	char *res = cmdline_get_value(key);

	if(!res)
		return (char *)default_value;

	return res;
}