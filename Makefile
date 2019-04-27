ARCH                   := i686-elf
CROSS_COMPILER         := $(ARCH)

export CROSS_COMPILER

BOOT                   := boot
ISO                    := iso

CWD                    := $(shell pwd)
SRC                    := $(CWD)/src

export SRC

DIR_ISO                := $(CWD)/$(ISO)
DIR_BOOT               := $(DIR_ISO)/boot
DIR_KERNEL             := $(DIR_ISO)/kernel

IMAGE_BOOT_SECTOR      := $(BOOT).bin
IMAGE_BOOT_LOADER      := $(BOOT)/$(IMAGE_BOOT_SECTOR)

ISO_BOOT_PATH          := $(DIR_BOOT)

export ISO_BOOT_PATH

ISO_BOOTSTRAP_PATH     := $(BOOTSTRAPDIR)
ISO_KERNEL_PATH        := $(KERNELDIR)

export ISO_BOOTSTRAP_PATH
export ISO_KERNEL_PATH

IMAGE                  := $(BOOT).iso

BOCHSRC                := bochsrc

.PHONY: all
all: $(IMAGE)

.PHONY: clean
clean:
	rm -rf $(DIR_ISO)
	rm -f *.ini
	rm -f *.iso
	$(MAKE) -C src/arch/x86/boot clean

.PHONY: isotree
isotree:
	isoinfo -f -i os.iso

$(IMAGE): build bootloader
	genisoimage -R -b $(IMAGE_BOOT_LOADER) -input-charset utf-8 -no-emul-boot -V CR0S -v -o $(IMAGE) $(DIR_ISO)

.PHONY: build
build:
	mkdir -p $(DIR_BOOT)
	mkdir -p $(DIR_KERNEL)
	cp lorem.txt $(DIR_BOOT)
	cp cmdline.txt $(DIR_ISO)
	cp boot.bin $(DIR_KERNEL)

.PHONY: bootloader
bootloader:
	$(MAKE) -C src/arch/x86/boot
