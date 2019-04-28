ELFBOOT := elfboot

ARCH    := x86
BITS    := 32

ELFBOOT_DIR   := $(PWD)

TARGET  := i686

TOOLCHAIN_DIR := $(ELFBOOT_DIR)/toolchain
PATH    := $(TOOLCHAIN_DIR)/cross/$(TARGET)-$(ELFBOOT)/bin:$(PATH)

CC      := $(TARGET)-$(ELFBOOT)-gcc
LD      := $(TARGET)-$(ELFBOOT)-ld

