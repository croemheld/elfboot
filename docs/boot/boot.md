# Boot

The following text describes the memory layout at boot time. The memory regions discussed here range from the physical addresses `0x00000000` to `0x00010000`. Everything located above `0x00010000` (`0x00010000` to `0xffffffff`) is not used by the bootloader but the actual kernel.

After the jump to the 32-bit kernel, the memory regions occupied by the bootloader are mostly overwritten. This includes the stack, the primary volume descriptor, boot sector as well as the stage-2 bootloader. The BIOS font bitmap is relocated to a different location before it is used by the kernel to make printing characters in VESA graphics modes possible.

## Stack layout

At boot time, the memory region between `0x00000000` and `0x00010000` is described as follows:

| Start Address | End Address  | Description |
|      :---:    |     :---:    |     :---    |
| `0x00000000`  | `0x00001800` | Boot Stack |
| `0x00001800`  | `0x00002000` | *Unused* |
| `0x00002000`  | `0x00002800` | Primary Volume Descriptor |
| `0x00002800`  | `0x00003000` | Temporary Directory |
| `0x00003000`  | `0x00004000` | File Buffer |
| `0x00004000`  | `0x00005000` | *Unused* |
| `0x00005000`  | `0x00006000` | *Unused* |
| `0x00006000`  | `0x00007000` | *Unused* |
| `0x00007000`  | `0x00008000` | Boot Sector |
| `0x00008000`  | `0x0000A000` | Stage-2 Bootloader |
| `0x0000A000`  | `0x0000B000` | *Unused* |
| `0x0000B000`  | `0x0000C000` | *Unused* |
| `0x0000C000`  | `0x0000D000` | *Unused* |
| `0x0000D000`  | `0x0000E000` | *Unused* |
| `0x0000E000`  | `0x0000F000` | *Unused* |
| `0x0000F000`  | `0x00010000` | *Unused* |