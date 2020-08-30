![elfboot](../images/elfboot-logo.png)

### Configurations ###

In the following paragraphs we will explain the possible configurations and what values are supported for these.

#### Debug configurations ####

##### `DEBUG` #####

 - Possible values: `y`, `n`
 - Enables general elfboot debug output to the screen.

##### `DEBUG_BOCHS` #####

 - Possible values: `y`, `n`
 - By enabling this configuration elfboot will use the Bochs emulator port 0xE9 hack to print debug information to the Bochs console.

##### `DEBUG_QEMU` #####

 - Possible values: `y`, `n`
 - Enables debug output to the QEMU console via serial port.

#### Architecture specific configurations ####

##### `X86_INTERRUPT_INFO` #####

 - Possible values: `y`, `n`
 - By enabling this configuration, elfboot will print interrupt information (CPU state, stack trace)

#### General configurations ####

##### `LOADER_AUTOBOOT` #####

 - Possible values: `y`, `n`
 - If this configuration is enabled, and the elfboot bootloader only finds one single boot entry in the `elfboot.cfg` boot file, it will skip the loader menu and directly boot the only available kernel. This speeds up the bootloader since it does not have to draw the loader menu.

#### Module configurations ####

Each module configuration supports 3 values: `y`, `n`, `m`. `y` means, the module will be built-in and statically linked at build time. `m` will make the module an external one so that it can be loaded at runtime. The module is linked dynamically during that process. Lastly, `n` means, the module will not be built at all.

Most of the times, each module configuration supports the additional configuration `*_DEBUG` which can either be `y` or `n` and enables or disables debug output specifically for this module.

##### `DRIVER_IDE` #####

 - Possible values: `y`, `m`, `n`
 - Module for the PCI IDE controller.

##### `DRIVER_AHCI` #####

 - Possible values: `y`, `m`, `n`
 - Module for the PCI AHCI/RAID controller.

##### `DRIVER_TTY` #####

 - Possible values: `y`, `m`, `n`
 - Module for the TTY device. elfboot is implemented in a way that it can work with and without this module. If this configuration is set to `n`, and there are multiple available boot entries found in `elfboot.cfg`, it will only boot the first one.

##### `DRIVER_PIT` #####

 - Possible values: `y`, `m`, `n`
 - Module for the Programmable Interval Timer (PIT). elfboot can be build without this module, too. Doing so will remove the counter for the boot menu in which elfboot automatically boots the selected entry if no key was pressed by the user.

##### `DRIVER_KBD` #####

 - Possible values: `y`, `m`, `n`
 - Module for the PS/2 keyboard. Mandatory if you want to be able to use the graphical boot menu for selecting other boot entries.

##### `FS_ISOFS` #####

 - Possible values: `y`, `m`, `n`
 - File system module for the *ISO 9660* file system. Mandatory if you want to be able to boot from CD.

##### `FS_EXT2FS` #####

 - Possible values: `y`, `m`, `n`
 - File system module for the *ext2* file system. Mandatory if you want to be able to boot from HDD or a flat disk image.