ELFBOOT := elfboot

ARCH    := x86
BITS    := 32

TARGET  := i686

export ELFBOOT
export TARGET

CC      := /home/croemheld/Repositories/elfboot/toolchain/cross/bin/$(TARGET)-$(ELFBOOT)-gcc
LD      := /home/croemheld/Repositories/elfboot/toolchain/cross/bin/$(TARGET)-$(ELFBOOT)-ld

CCFLAGS  := -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  := -O2 -nostdlib -lgcc

all: test.c
	$(CC) -c test.c -o test.o $(CCFLAGS)
	$(LD) test.o -o test.bin $(LDFLAGS)

.PHONY: toolchain
toolchain: toolchain/gentoolchain.sh
	$(MAKE) -C toolchain

.PHONY: clean-toolchain
clean-toolchain:
	$(MAKE) -C toolchain clean