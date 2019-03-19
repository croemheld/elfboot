#include <boot/boot.h>
#include <boot/disk.h>
#include <boot/linkage.h>

.code16

	.extern _PARTITION_CHS_HEAD
	.extern _PARTITION_CHS_SECTOR
	.extern _PARTITION_CHS_CYLINDER
	.extern _PARTITION_BLOCKS

.section .rodata, "a", @progbits

bios_error_msg:
	.asciz "BIOS function error\r\n"

a20_error_msg:
	.asciz "Could not enable A20 line\r\n"

.section .data, "aw", @progbits

	.global boot_params
	.global boot_params_disk_drive
	.global boot_params_e820_count
	.global boot_params_e820_table
	.global boot_params_code_entry

boot_params:

#
# Boot relevant information
#

boot_params_disk_drive: .word 0x0
boot_params_e820_count: .word 0x0
boot_params_code_entry: .long 0x01000000
boot_params_e820_table: .skip 320, 0x00

#
# This section contains the boot sector. We place it at the dedicated .boot 
# section of the binary whereas all other code is located after the first 
# 512 bytes.
#

.section .boot, "aw", @progbits

	.global _start

#
# Bootloader entry point
#

_start:
	ljmp $0, $_setup16

#
# The actual code after clearing %cs
#

_setup16:
	cli

	xorw    %ax, %ax
	movw    %ax, %ss
	movw    %ax, %ds
	movw    %ax, %es

	movw    $BOOT_STACK_ADDR, %sp

	# Clear screen and reset text mode cursor

	call    clear_screen

	jmp     _setup_bmain

.org 0x1BE

	# Dummy MBR entries to statisfy some BIOSes

	.byte 0x80
	.byte 0x00
	.byte 0x01
	.byte 0x00
	.byte 0xda

	.byte _PARTITION_CHS_HEAD
	.byte _PARTITION_CHS_SECTOR_BYTE
	.byte _PARTITION_CHS_CYLINDER_BYTE

	.long 0x00
	.long _PARTITION_BLOCKS

.org 0x1CE
.org 0x1DE
.org 0x1EE

.section .text

_setup_bmain:
	cli

	xorw    %ax, %ax
	movw    %ax, %ds

	# Save boot parameters

	movw    %dx, boot_params_disk_drive

	# Enable A20 line 

	call    enable_a20
	jc      .a20_error

	# Finally, call bootloader main routine

	movw    $boot_params, %ax
	call    bmain

# .die:
#
# Halts the system by entering an infinite loop.

GLOBAL(.die)
	jmp     .die
ENDPROC(.die)

# .a20_error:
#
# Prints an error message to indicate the lack 
# of A20 support before halting the system.

GLOBAL(.a20_error)
	movw    $a20_error_msg, %si
	call    bprintf

	jmp     .die
ENDPROC(.a20_error)

# .bios_error
#
# Halts the system and puts an error message on
# the screen before entering an infinite loop.

GLOBAL(.bios_error)

	# Clear modifications made to the registers

	xorw    %ax, %ax
	movw    %ax, %ds

	movw    $bios_error_msg, %si
	call    bprintf

	jmp     .die
ENDPROC(.bios_error)
