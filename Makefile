ELFBOOT := elfboot

ARCH    := x86
BITS    := 32

TARGET  := i686

export ELFBOOT
export TARGET

CROSS_COMPILER_PATH := $(PWD)/toolchain/$(TARGET)-$(ELFBOOT)/bin

export PATH := $(CROSS_COMPILER_PATH):$(PATH)

CC       := $(TARGET)-$(ELFBOOT)-gcc

CFLAGS   := -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  := -O2 -nostdlib -lgcc

INCLUDE  := -Isrc/arch/$(ARCH)/include \
	    -Isrc/include

objtree  := .
srctree  := .

.PHONY: all
all: real-all

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
$(eval $(call build_subdir,src/arch/$(ARCH)))

define compile_file
srcfile := $$(wildcard $$(basename $(1)).*)

ifneq ($$(srcfile),"")

$(1): $$(srcfile)
	@echo "[ CC ] $$@"
# $(CC) -c $$(srcfile) -o $(1) $(CFLAGS) $(INCLUDE)

endif
endef

$(foreach file,$(OBJS),$(eval $(call compile_file,$(file))))

.PHONY: real-all
real-all: $(OBJS)
	@echo "real-all"
# $(CC) test.c -o test.o $(CFLAGS)

.PHONY: clean
clean:
	echo "Bye"

.PHONY: test
test:
	echo "Test"

.PHONY: toolchain
toolchain: toolchain/gentoolchain.sh
	$(MAKE) -C toolchain

.PHONY: clean-toolchain
clean-toolchain:
	$(MAKE) -C toolchain clean