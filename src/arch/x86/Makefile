elfboot-y += a20.o
elfboot-y += acpi.o
elfboot-y += bda.o
elfboot-y += bios.o
elfboot-y += boot.o

# The GCC compiler does not seem to use 32-bit 
# offsets for the rep; movsb/w/l instructions, 
# which makes this entire file obsolete.
# 
# The implementations of the functions in this 
# file are stored in src/core/string.c.

#elfboot-y += copy.o

elfboot-y += edd.o
elfboot-y += e820.o
elfboot-y += main.o
elfboot-y += pic.o
elfboot-y += pmjump.o
elfboot-y += video.o
elfboot-y += video-vesa.o