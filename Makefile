ELFBOOT := elfboot

ELFBOOT_ARCH   := x86
ELFBOOT_BITS   := 32

ELFBOOT_TARGET := i686
ELFBOOT_TCHAIN := elfboot-toolchain

ELFBOOT_ISO     := $(CURDIR)/iso
ELFBOOT_BOOT    := boot
ELFBOOT_ISOBOOT := $(ELFBOOT_ISO)/$(ELFBOOT_BOOT)

ELFBOOT_BINARY  := $(ELFBOOT_BOOT)/$(ELFBOOT).bin

# Export the modified PATH variable
export PATH  := $(CURDIR)/$(ELFBOOT_TCHAIN)/bin:$(PATH)
export SHELL := env PATH=$(PATH) /bin/bash

# Export relevant variables needed for the toolchain
export ELFBOOT_ARCH
export ELFBOOT_TARGET

# Programs for compiling and linking
CC := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc
LD := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc
PY := python3

# Flags for compiler and linker
CFLAGS  := -std=gnu99 -ffreestanding -m16 -Wextra -g -O3	\
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
elfboot: elfboot-config toolchain-check $(OBJS)
	@echo "  GENISO  $(ELFBOOT).iso"
	@mkdir -p $(ELFBOOT_ISOBOOT)
	@$(LD) -o $(ELFBOOT).bin -T $(ELFBOOT).ld $(OBJS) $(LDFLAGS)
	@cp $(ELFBOOT).bin $(ELFBOOT_ISOBOOT)
	@genisoimage -R -b $(ELFBOOT_BINARY)				\
		-input-charset utf-8					\
		-no-emul-boot						\
		-V elfboot						\
		-v -o $(ELFBOOT).iso $(ELFBOOT_ISO)

elfboot-config:
	@$(PY) tools/genconf.py -i $(ELFBOOT).config			\
			       -o src/include/elfboot/config.h

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
	@rm -f $(ELFBOOT).bin $(ELFBOOT).iso
	@rm -rf $(ELFBOOT_ISO)
	@for objfile in $(OBJS); do			\
		if [[ -e $$objfile ]]; then		\
			echo "  CLEAN   $$objfile";	\
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