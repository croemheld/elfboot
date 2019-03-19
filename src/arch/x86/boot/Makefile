CC      := $(CROSS_COMPILER)-gcc
LD      := $(CROSS_COMPILER)-ld

DIR     := $(subst $(SRC), , $(shell pwd))

CFLAGS  := -ffreestanding -Wall -Wstrict-prototypes -m16 -Os                   \
		   -march=i386 -mregparm=3 -mpreferred-stack-boundary=2                \
		   -fno-strict-aliasing -fomit-frame-pointer -fno-pic                  \
		   -mno-mmx -mno-sse -fno-stack-protector

BOOT    := boot
OBJS    := boot.o a20.o bios.o bmain.o cmdline.o copy.o edd.o memory.o         \
           pmjump.o printf.o regs.o screen.o string.o tty.o iso.o video.o

INCLUDE := -I$(SRC)/include
INCLUDE += -I$(SRC)/arch/x86/include

.PHONY: all
all: $(BOOT).bin

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.bin

$(BOOT).bin: $(OBJS)
	@echo "Building $@..."
	@$(LD) -T $(BOOT).ld $(OBJS) -o $@
	@cp $@ $(ISO_BOOT_PATH)
	@echo "Successfully built $@"

%.o: %.S
	@echo "[ CC ] $(DIR)/$@"
	@$(CC) -c $< -o $@ $(INCLUDE)

%.o: %.c
	@echo "[ CC ] $(DIR)/$@"
	@$(CC) -c $< -o $@ $(INCLUDE) $(CFLAGS)