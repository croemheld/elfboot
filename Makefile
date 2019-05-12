ELFBOOT := elfboot

ELFBOOT_ARCH   := x86
ELFBOOT_BITS   := 32

ELFBOOT_TARGET := i686
ELFBOOT_TCHAIN := elfboot-toolchain

ELFBOOT_CROSS_COMPILER := $(shell command -v $(CC) 2> /dev/null)

# Export the modified PATH variable
export PATH := $(CURDIR)/$(ELFBOOT_TCHAIN)/bin:$(PATH)

# Export relevant variables needed for the toolchain
export ELFBOOT_ARCH
export ELFBOOT_TARGET

# Programs for compiling and linking
CC := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc
LD := $(ELFBOOT_TARGET)-$(ELFBOOT)-gcc

# Flags for compiler and linker
CFLAGS  := -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS := -O2 -nostdlib -lgcc

# Includes
ELFBOOT_INCLUDE := -Isrc/include			\
		   -Isrc/arch/$(ELFBOOT_ARCH)/include

# Source and object file tree
objtree  := .
srctree  := .

PHONY :=
CLEAN :=

PHONY += all
all: build

OBJS     :=

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

$(eval $(call build_subdir,src))
$(eval $(call build_subdir,src/arch/$(ELFBOOT_ARCH)))

define compile_file
srcname := $$(basename $(1))
srcfile := $$(wildcard $$(srcname).*)

ifneq (, $$(srcfile))

$(1): $$(srcfile)
	@echo "[ CC ] $$@"
	@$$(CC) -c $$< -o $$@ $$(CFLAGS) $$(ELFBOOT_INCLUDE)

endif
endef

$(foreach file,$(OBJS),$(eval $(call compile_file,$(file))))

PHONY += build
build: toolchain-check $(OBJS)
	@echo "build"

PHONY += toolchain
toolchain:
	$(MAKE) -C $(ELFBOOT_TCHAIN)

PHONY += toolchain-check
toolchain-check:
ifndef ELFBOOT_CROSS_COMPILER
	$(error "Please run 'make toolchain' first.")
endif

PHONY += clean-elfboot
CLEAN += clean-elfboot
clean-elfboot:
	@for objfile in $(OBJS); do			\
		if [[ -e $$objfile ]]; then		\
			@echo "[ RM ] $$file";		\
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