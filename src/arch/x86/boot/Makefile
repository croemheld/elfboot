#
# General boot compiler options
#

BOOTIMG_CFLAGS := -m16
BOOTIMG_CFLAGS += -Wno-unused-macros
BOOTIMG_CFLAGS += -fomit-frame-pointer
BOOTIMG_CFLAGS += -fno-stack-protector

#
# Boot image objects and compiler options
#

bootimg-y += boot.o

bootimg-cflags += $(BOOTIMG_CFLAGS)