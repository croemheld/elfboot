elfboot-d += arch
elfboot-d += core
elfboot-d += crypto
elfboot-d += drivers
elfboot-d += fs
elfboot-d += mm

elfboot-cflags += -Wno-unused-variable
elfboot-cflags += -Wno-unused-function
elfboot-cflags += -Wno-unused-parameter
elfboot-cflags += -Wno-unused-but-set-variable
elfboot-cflags += -fomit-frame-pointer
elfboot-cflags += -fno-stack-protector

elfboot-cflags += -mpreferred-stack-boundary=2

# CScope is only able to run in the directory 
# it is called from. Therefore we make a rule
# here in order to prevent cscope from inclu-
# ding the whole toolchain.
# 
# The actual cscope.out file is then moved to
# the projects root directory.

# cscope:
# 	cscope -R
