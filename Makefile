#
# elfboot main Makefile
#

ELFBOOT := elfboot

ELFBOOT_TARGET := i686
ELFBOOT_ARCH   := x86

export ELFBOOT ELFBOOT_TARGET ELFBOOT_ARCH

#
# Toolchain
#

ELFBOOT_TCHAIN := elfboot-toolchain
export PATH  := $(CURDIR)/$(ELFBOOT_TCHAIN)/bin:$(PATH)
export SHELL := env PATH=$(PATH) /bin/bash

#
# Verbose output
#

ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

export quiet Q VERBOSE

srctree	:= .
objtree	:= .

#
# Targets
#

BOOTUID			:= bootuid
DOTCONF			:= $(ELFBOOT).config
GENCONF			:= src/include/elfboot/config.h
AUTOCONF		:= auto.conf

IMGCONF			:= $(DOTCONF) $(AUTOCONF) $(GENCONF)

MODULES			:= modules

ELFTOOL_TARGETS	:= tools

ELFBOOT_PREREQS := $(ELFTOOL_TARGETS) $(IMGCONF) toolchain-check
ELFBOOT_ROOTDIR := 
ELFBOOT_BITSIZE := $(ELFBOOT)_bin_len
ELFBOOT_BFDSIZE := $(BOOTUID)_bfd_len

BOOTIMG			:= bootimg
BOOTIMG_PREREQS := $(ELFTOOL_TARGETS) $(IMGCONF) toolchain-check
BOOTIMG_ROOTDIR := arch/$(ELFBOOT_ARCH)/boot

BOOTISO			:= $(ELFBOOT).iso

#
# elfboot logo TMG file
#

EBIMAGE			:= $(ELFBOOT)

# 
# Boot image
#

BOOTISO_CDFLAGS += -b $(BOOTIMG).bin
BOOTISO_CDFLAGS += -no-emul-boot
BOOTISO_CDFLAGS += -volid ELFBOOT
BOOTISO_CDFLAGS += -boot-info-table

#
# Weight sorting
#

BOOTISO_CDFLAGS += --sort-weight 6 $(BOOTIMG).bin
BOOTISO_CDFLAGS += --sort-weight 5 $(ELFBOOT).bin
BOOTISO_CDFLAGS += --sort-weight 4 $(ELFBOOT).bid
BOOTISO_CDFLAGS += --sort-weight 3 $(ELFBOOT).tmg
BOOTISO_CDFLAGS += --sort-weight 2 $(ELFBOOT).map
BOOTISO_CDFLAGS += --sort-weight 1 $(MODULES)

#
# Additional options
#

BOOTISO_CDFLAGS += -full-iso9660-filenames

#
# Recipes for targets
#

PHONY += all
all: toolchain-check
	@echo " Please use one of the following targets:"
	@echo " $(BUILD)"

include $(srctree)/Makefile.lib

BUILD += $(ELFBOOT)
$(ELFBOOT): $(ELFBOOT_PREREQS)
	$(Q)$(call compile,$(ELFBOOT),$(ELFBOOT_ROOTDIR))
	$(Q)$(call modules,$(ELFBOOT),$(ELFBOOT_ROOTDIR))
	$(Q)$(call objcopy,$(ELFBOOT))

$(ELFBOOT).map: $(ELFBOOT)
	$(Q)$(NM) $(NMFLAGS) $(ELFBOOT).elf > $@

BUILD += auto.conf
auto.conf: $(DOTCONF)
	$(Q)$(call builtin,$(DOTCONF))

BUILD += $(BOOTIMG)
$(BOOTIMG): $(BOOTIMG_PREREQS)
	$(Q)$(call compile,$(BOOTIMG),$(BOOTIMG_ROOTDIR))
	$(Q)$(call elfconf,$(BOOTIMG),$(ELFBOOT_BITSIZE),$(ELFBOOT).bin)
	$(Q)$(call elfconf,$(BOOTIMG),$(ELFBOOT_BFDSIZE),$(ELFBOOT).bid)
	$(Q)$(call objcopy,$(BOOTIMG))

BUILD += $(ELFTOOL_TARGETS)
PHONY += $(ELFTOOL_TARGETS)
$(ELFTOOL_TARGETS):
	$(Q)$(MAKE) -C $(ELFTOOL_TARGETS)

BUILD += $(GENCONF)
$(GENCONF): $(ELFBOOT).config
	$(Q)$(call genconf)

PHONY += toolchain
BUILD += toolchain
toolchain:
	$(Q)$(MAKE) ELFBOOT_TARGET=$(ELFBOOT_TARGET) -C $(ELFBOOT_TCHAIN)

PHONY += toolchain-check
toolchain-check:
ifeq (, $(shell type $(CC) 2> /dev/null))
	$(error Please run 'make toolchain' first)
endif

build:
	$(Q)mkdir -p build/$(MODULES)

iso: build $(ELFBOOT) $(ELFBOOT).map $(BOOTIMG)
	$(Q)$(CP) $(BOOTIMG).bin build
	$(Q)$(CP) $(ELFBOOT).bin build
	$(Q)$(CP) $(ELFBOOT).bid build
	$(Q)$(CP) $(ELFBOOT).cfg build
	$(Q)$(CP) $(ELFBOOT).tmg build
	$(Q)$(CP) $(ELFBOOT).map build
	@echo "  XORRISO $(BOOTISO)"
	$(Q)$(XR) $(BOOTISO_CDFLAGS) -o $(BOOTISO) build 2> /dev/null

#
# Clean build
#

PHONY += clean-$(ELFBOOT)
CLEAN += clean-$(ELFBOOT)
clean-$(ELFBOOT):
	$(Q)$(call cleanup,$(ELFBOOT),$(ELFBOOT_ROOTDIR))

PHONY += clean-$(GENCONF)
CLEAN += clean-$(GENCONF)
clean-$(GENCONF):
	$(Q)$(RM) -f src/include/elfboot/config.h

PHONY += clean-auto.conf
CLEAN += clean-auto.conf
clean-auto.conf:
	$(Q)$(RM) -f auto.conf

PHONY += clean-$(BOOTIMG)
CLEAN += clean-$(BOOTIMG)
clean-$(BOOTIMG):
	$(Q)$(call cleanup,$(BOOTIMG),$(BOOTIMG_ROOTDIR))

PHONY += clean-$(ELFTOOL_TARGETS)
CLEAN += clean-$(ELFTOOL_TARGETS)
clean-$(ELFTOOL_TARGETS):
	@echo "  CLEAN   $(ELFTOOL_TARGETS)"
	$(Q)$(MAKE) -C $(ELFTOOL_TARGETS) clean

PHONY += clean-toolchain
CLEAN += clean-toolchain
clean-toolchain:
	@echo "  CLEAN   $(ELFBOOT_TCHAIN)"
	$(Q)$(MAKE) -C $(ELFBOOT_TCHAIN) clean

PHONY += clean-iso
CLEAN += clean-iso
clean-iso: clean-$(BOOTIMG) clean-$(ELFBOOT) clean-auto.conf clean-$(GENCONF)
	@echo "  CLEAN   $(BOOTISO)"
	$(Q)$(RM) -f $(MODULES).lst
	$(Q)$(RM) -f $(ELFBOOT).map
	$(Q)$(RM) -f $(ELFBOOT).iso

PHONY += clean-build
CLEAN += clean-build
clean-build: clean-$(ELFTOOL_TARGETS) clean-iso
	$(Q)$(RM) -rf build

PHONY += clean
clean:
	@echo " Please use one of the following targets:"
	@echo " $(CLEAN)"

.PHONY: $(PHONY)