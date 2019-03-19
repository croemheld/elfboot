AS                                                := as
CC                                                := gcc
LD                                                := ld

ISODIR                                            := iso
BOOTDIR                                           := boot
SETUPDIR                                          := setup
BOOTPATH                                          := $(ISODIR)/$(BOOTDIR)

BOOT                                              := boot
BOOTBIN                                           := $(BOOTDIR)/$(BOOT).bin

SETUP                                             := setup
SETUPBIN                                          := $(SETUPDIR)/$(SETUP).bin

LOADERISO                                         := $(BOOT).iso

BOOTIMG                                           := $(BOOTDIR)/$(BOOTBIN)

BOCHSRC                                           := bochsrc

BOOTOBJ  := boot.o print.o screen.o disk.o
SETUPOBJ := setup.o print.o screen.o disk.o

.PHONY: all
all: $(LOADERISO)

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.ini
	rm -f *.iso
	rm -rf iso/

.PHONY: isotree
isotree:
	isoinfo -f -i os.iso

$(LOADERISO): $(BOOTBIN) $(SETUPBIN)
	mkdir -p $(BOOTPATH)
	cp $(BOOTBIN) $(BOOTPATH)
	cp $(SETUPBIN) $(BOOTPATH)
	cp test.txt $(BOOTPATH)

	genisoimage -R -b $(BOOTIMG) -no-emul-boot -V CR0S -v -o $(LOADERISO) $(ISODIR)

$(BOOTBIN): $(BOOTOBJ)
	$(LD) -T boot.ld $(BOOTOBJ) -o $@

$(SETUPBIN): $(SETUPOBJ)
	$(LD) -T setup.ld $(SETUPOBJ) -o $@

%.o: %.asm
	$(AS) $< -o $@