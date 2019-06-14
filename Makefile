ELFBOOT := elfboot

ELFBOOT_ARCH   := x86
ELFBOOT_BITS   := 32

ELFBOOT_TARGET := i686
ELFBOOT_TCHAIN := elfboot-toolchain

# Export the modified PATH variable
export PATH  := $(CURDIR)/$(ELFBOOT_TCHAIN)/bin:$(PATH)
export SHELL := env PATH=$(PATH) /bin/bash

# Export relevant variables needed for the toolchain
export ELFBOOT_ARCH
export ELFBOOT_TARGET

# Programs for compiling and linking
CC := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc
LD := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc

# Flags for compiler and linker
CFLAGS  := -std=gnu99 -ffreestanding -m16 -Wextra -g -Os	\
	   -Wall -Wstrict-prototypes -march=i386 -mregparm=3	\
	   -fno-strict-aliasing -fomit-frame-pointer -fno-pic	\
	   -mno-mmx -mno-sse
LDFLAGS := -O2 -nostdlib -lgcc

# Includes
ELFBOOT_INCLUDE := -Isrc/include			\
		   -Isrc/arch/$(ELFBOOT_ARCH)/include

# Recipe variables
PHONY :=
CLEAN :=
BUILD :=

PHONY += all
all: toolchain-check
	@echo " Please use one of the following targets:"
	@echo " $(BUILD)"


OBJS     :=

# Source and object file tree
objtree  := .
srctree  := .

define build_subdir
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

elfboot-y :=
elfboot-d :=

include $$(srctree)/Makefile

OBJS += $$(patsubst %,$$(objtree)/%,$$(elfboot-y))

$$(foreach subdir,$$(elfboot-d),$$(eval $$(call build_subdir,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef

$(eval $(call build_subdir,src/arch/$(ELFBOOT_ARCH)))
$(eval $(call build_subdir,src))

define compile_file
srcname := $$(basename $(1))
sofiles := $$(wildcard $$(srcname).*)
srcfile := $$(filter-out $(1),$$(sofiles))

ifneq (, $$(srcfile))

$(1): $$(srcfile)
	@echo "  CC      $$@"
	@$$(CC) -c $$< -o $$@ $$(CFLAGS) $$(ELFBOOT_INCLUDE)

endif
endef

$(foreach file,$(OBJS),$(eval $(call compile_file,$(file))))

PHONY += elfboot
BUILD += elfboot
elfboot: clean-elfboot toolchain-check $(OBJS)
	@echo "  LD      $(OBJS)"
	@$(LD) -o $(ELFBOOT).bin -T $(ELFBOOT).ld $(OBJS) $(LDFLAGS)

PHONY += toolchain
BUILD += toolchain
toolchain:
	$(MAKE) -C $(ELFBOOT_TCHAIN)

PHONY += toolchain-check
toolchain-check:
ifeq (, $(shell type $(CC) 2> /dev/null))
	$(error Please run 'make toolchain' first)
endif

PHONY += clean-elfboot
CLEAN += clean-elfboot
clean-elfboot:
	@for objfile in $(OBJS); do			\
		if [[ -e $$objfile ]]; then		\
			echo "  CLEAN   $$objfile";		\
			rm -f $$objfile;		\
		fi					\
	done

PHONY += clean-toolchain
CLEAN += clean-toolchain
clean-toolchain:
	@echo "Clean $(ELFBOOT_TCHAIN)..."
	$(MAKE) -C $(ELFBOOT_TCHAIN) clean

PHONY += clean
clean:
	@echo " Please use one of the following targets:"
	@echo " $(CLEAN)"

.PHONY: $(PHONY)