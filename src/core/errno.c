#include <elfboot/core.h>
#include <elfboot/string.h>
#include <elfboot/printf.h>

#include <uapi/elfboot/errno.h>

/*
 * List of error messages for all errors listed in
 * include/uapi/elfboot/errno.h. A value of 0 means
 * that there was no error and the result indicates
 * a success.
 */

static const char *errno_messages[] = {
	"Success"
	"Not owner",
	"No such file or directory",
	"No such context",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Arg list too long",
	"Exec format error",
	"Bad file number",
	"No children",
	"No more contexts",
	"Not enough core",
	"Permission denied",
	"Bad address",
	"Directory not empty",
	"Mount device busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"File name too long",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Resource deadlock avoided",
	"No locks available",
	"Unsupported value",
	"Message size",
};

void bprintln_error(int errno)
{
	if (abs(errno) >= ARRAY_SIZE(errno_messages))
		bprintln("Failed with unknown error %d", errno);

	/*
	 * As error numbers are intended to be values lower than 0,
	 * we need to negate the errno parameter value to get the
	 * correct index for our error message.
	 */
	
	bprintln("Failed with error %d: %s", errno, errno_messages[-errno]);
}