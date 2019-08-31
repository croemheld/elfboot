# elfboot #

A multiboot-compliant *Executable and Linking Format* (ELF) bootloader for the x86 architecture.

### Build ###

To clone the repository and all its submodules for the toolchain, type:

```shell
$ git clone --recursive git@github.com:croemheld/elfboot.git
```

First, we need to build the toolchain for the bootloader, which is simply done with:

```shell
$ make toolchain
```

The toolchain can now be used to compile the bootloader via:

```shell
$ make elfboot
```