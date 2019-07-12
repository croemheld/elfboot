elfboot-d += core
elfboot-d += drivers
elfboot-d += fs
elfboot-d += mm

# CScope is only able to run in the directory 
# it is called from. Therefore we make a rule
# here in order to prevent cscope from inclu-
# ding the whole toolchain.
# 
# The actual cscope.out file is then moved to
# the projects root directory.

cscope:
	cscope -R