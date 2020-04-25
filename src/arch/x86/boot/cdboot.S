#include <asm/boot.h>
#include <asm/segment.h>
#include <asm/linkage.h>

.code16

# Address where the PVD is loaded

#define PVD_BUFFER_SEGMENT			0x0200

# Buffer address for temporary directories when searching for files

#define DIR_BUFFER_SEGMENT			0x0280

# Offsets within the PVD structure

#define PVD_TYPE_CODE_OFFSET		  0
#define PVD_ROOT_DIRECTORY_OFFSET	156

# Constants

#define PVD_TYPE_PVD				  1

# ISO 9660 directory fields

#define DIR_LENGTH_OFFSET			  0
#define DIR_EXTR_SEC_OFFSET			  2
#define DIR_DATA_LEN_OFFSET			 10
#define DIR_FILE_ID_LENGTH_OFFSET	 32
#define DIR_FILE_ID_OFFSET			 33

.section .text

# read_pvd:
# 
# Loads the Primary Volume Descriptor (PVD) into the memory region
# at $PVD_BUFFER_SEGMENT.

LOCAL(read_pvd)
	movw    $0x0010, %bx
	movw    $0x0001, %cx

	# Store the PVD in the designed buffer

	movw    $PVD_BUFFER_SEGMENT, %ax
	movw    %ax, %es

.find_pvd_loop:
	call    read_sector

	cmpb    $PVD_TYPE_PVD, %es:PVD_TYPE_CODE_OFFSET
	je      .pvd_found
	inc     %bx

	jmp     .find_pvd_loop

.pvd_found:
	ret
ENDPROC(read_pvd)

# read_root_dir:
#
# Stores the root directory located in the PVD at the address specified
# by the segment value stored in %ax.
#
# Parameters:
#
#     - %ax: Segment value where the root directory should be stored at

LOCAL(read_root_dir)
	
	# Segment %es is always pointing to the beginning of the PVD buffer

 	movl    %es:PVD_ROOT_DIRECTORY_OFFSET + DIR_EXTR_SEC_OFFSET, %ebx
 	movl    %es:PVD_ROOT_DIRECTORY_OFFSET + DIR_DATA_LEN_OFFSET, %ecx

 	# Number of sectors equals data length divided by 2^11 (2048)

	shr     $11, %ecx

 	call    read_sector

 	ret
ENDPROC(read_root_dir)

#
# Read next stage from CDROM into memory
#

GLOBAL(read_second_stage)
	lea     second_stage_id, %ax
	movw    $SECOND_STAGE_SEGMENT, %di
	call    read_entry

	lea     elfboot_logo, %ax
	movw    $ELFBOOT_LOGO_SEGMENT, %di
	call    read_entry

	ret
ENDPROC(read_second_stage)

# read_entry:
# 
# Locates a file on the disk and loads it at the
# address specified by %di.
# 
# Parameters:
#
#     - %ax: Address of the full path to the file
#     - %di: Destination segment where to load the file

GLOBAL(read_entry)
	push    %es
	push    %di

	# Store address parameter

	push    %ax
	call    read_pvd

	# Store destination parameter

	movw    $DIR_BUFFER_SEGMENT, %ax
	push    %ax
	call    read_root_dir

	# %es now points to the root directory

	pop     %es
	pop     %si

	# This function expects the path to be a full path, i.e. starting with '/'

	inc     %si

.read_entry_dir_loop:
	xorw    %bx, %bx
	push    %si

.read_entry_strlen:
	
	# Determine length of the current path component

	cmpb    $0, (%si)
	je      .read_file

	cmpb    $'/', (%si)
	jne     .entry_next_char

	test    %bl, %bl
	jnz     .read_directory

	jmp     .die

.entry_next_char:

	# Get the next character in the full path

	inc     %bl
	inc     %si

	jmp     .read_entry_strlen

.read_file:

	# We are reading in a file from the disk

	movb     $1, %bh

.read_directory:

	# Read current directory entry

	pop     %ax
	push    %si

	call    find_directory_entry
 
 	test    %ax, %ax
 	jz      .die

 	test    %bh, %bh
 	jz      .load_directory

 	# If the current entry describes a file, set the address

.load_file:
	
	movw    %si, %di
	pop     %si

 	pop     %ax

 	push    %si
 	movw    %di, %si

 	jmp     .load_entry

.load_directory:

 	# If the current entry describes a directory, overwrite it

 	movw    %es, %ax

.load_entry:
 	push    %bx

	# Read entry

	movl    %es:DIR_EXTR_SEC_OFFSET(%si), %ebx
 	movl    %es:DIR_DATA_LEN_OFFSET(%si), %ecx

 	# Adjust number of bytes to read

 	cmpw    $0x0800, %cx
 	jae     .read_directory_sector

 	addw    $0x07ff, %cx

.read_directory_sector:

	shr     $11, %ecx
 
 	call    read_sector

 	# Prepare to traverse the next entry

 	pop     %bx
 	pop     %si

 	test    %bh, %bh
 	jnz     .entry_found

 	inc     %si

 	jmp     .read_entry_dir_loop

.entry_found:
	pop     %es

	ret
ENDPROC(read_entry)

# find_directory_entry:
# 
# Locates a file or directory within the current directory described in
# the buffer located at $DIR_BUFFER_SEGMENT.
# 
# Parameters:
# 
#     - %ax: The address of the name to search for
#     - %bl: The length of the name

LOCAL(find_directory_entry)
	push    %di

	movw    %ax, %di
	xorw    %ax, %ax
	movw    %ax, %si

.find_directory_entry_loop:
	cmpb    $0, %es:DIR_LENGTH_OFFSET(%si)
	je      .directory_entry_return

	# Check file name size

	xorw    %cx, %cx
	movb    %es:DIR_FILE_ID_LENGTH_OFFSET(%si), %cl

	cmpb    %bl, %cl
	jne     .directory_next_entry

	xorw    %bp, %bp

.directory_strcmp_loop:

	# %cl contains the character from the string

	movb    %ds:(%bp,%di), %cl

	# %ch contains the character from the directory entry

	movb    %es:DIR_FILE_ID_OFFSET(%bp,%si), %ch

	test    %cl, %cl
	jz      .directory_strcmp_check

	cmpb    $'/', %cl
	je      .directory_strcmp_check

	cmpb    %ch, %cl
	jne     .directory_next_entry

	inc     %bp

	jmp     .directory_strcmp_loop

.directory_strcmp_check:
	test    %ch, %ch
	jz      .directory_entry_found

	movw    %bp, %cx
	cmpb    %es:DIR_FILE_ID_LENGTH_OFFSET(%si), %cl
	je      .directory_entry_found

.directory_next_entry:
	xorw    %cx, %cx

	# Move on to next directory entry

	movb    %es:DIR_LENGTH_OFFSET(%si), %cl
	addw    %cx, %si

	jmp     .find_directory_entry_loop

.directory_entry_found:
	movw    $1, %ax

.directory_entry_return:
	pop     %di

	ret
ENDPROC(find_directory_entry)

# read_sector:
#
# Reads from the disk and stores it into the 
# buffer specified by the %es segment.
#
# Parameters:
# 
#     - %ax: Segment of buffer to write to
#     - %bx: Lower 32 bit of starting LBA
#     - %cx: Number of sectors to read

LOCAL(read_sector)
	lea     disk_address_packet, %si
	inc     %cx

	movw    %ax, 0x6(%si)
	movw    %bx, 0x8(%si)
	movw    %cx, 0x2(%si)

	movb    $0x42, %ah
	int     $0x13
	jc      .bios_error

	ret
ENDPROC(read_sector)

.section .rodata, "a", @progbits

	.align 0x10

# Disk Address Packet (DAP)

disk_address_packet:
	.word	0x0010
dap_ptr_cnum:
	.word	0
dap_ptr_dest:
	.long	0
dap_ptr_llba:
	.long	0

.size dap_ptr_llba, . - dap_ptr_llba

dap_ptr_hlba:
	.long	0

.size disk_address_packet, . - disk_address_packet
