elfboot-d += arch
elfboot-d += core
elfboot-d += crypto
elfboot-d += drivers
elfboot-d += fs
elfboot-d += lib
elfboot-d += loaders
elfboot-d += mm

elfboot-cflags += -Wunused-variable
elfboot-cflags += -Wunused-function
elfboot-cflags += -Wunused-parameter
elfboot-cflags += -Wno-unused-macros

elfboot-cflags += -fno-stack-protector
elfboot-cflags += -fno-strict-aliasing
elfboot-cflags += -fno-asynchronous-unwind-tables

elfboot-cflags += -fomit-frame-pointer

elfboot-cflags += -mno-red-zone -mno-mmx 
elfboot-cflags += -mno-sse -mno-sse2
elfboot-cflags += -mno-3dnow -mno-avx

ifneq ($(LLVM),)
elfboot-cflags += -mstack-alignment=4
else
elfboot-cflags += -mpreferred-stack-boundary=2
endif

# CScope is only able to run in the directory 
# it is called from. Therefore we make a rule
# here in order to prevent cscope from inclu-
# ding the whole toolchain.
# 
# The actual cscope.out file is then moved to
# the projects root directory.

# cscope:
# 	cscope -R
