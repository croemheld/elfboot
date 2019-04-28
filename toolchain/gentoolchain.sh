#!/usr/bin/env bash

TOOLCHAIN_DIR="$(pwd)"

CROSS_RESOURCE="https://ftp.gnu.org/gnu"

if [[ "$#" != 1 ]]; then
	echo "No target defined, abort..."
	exit 1
fi

CROSS_COMPILER_ROOT="$TOOLCHAIN_DIR/cross"
CROSS_COMPILER_TARGET="$1-elfboot"
CROSS_COMPILER_PREFIX="$CROSS_COMPILER_ROOT/$CROSS_COMPILER_TARGET"

export CROSS_COMPILER_TARGET
export CROSS_COMPILER_PREFIX
export PATH="$CROSS_COMPILER_PREFIX/bin:$PATH"

CROSS_BINVERSION=2.31.1
CROSS_BINPACKET="binutils-$CROSS_BINVERSION"
CROSS_BINREMOTE="$CROSS_RESOURCE/binutils/$CROSS_BINPACKET.tar.gz"

CROSS_GCCVERSION=8.2.0
CROSS_GCCPACKET="gcc-$CROSS_GCCVERSION"
CROSS_GCCREMOTE="$CROSS_RESOURCE/gcc/gcc-$CROSS_GCCVERSION/$CROSS_GCCPACKET.tar.gz"

mkdir -p $CROSS_COMPILER_PREFIX
cd $CROSS_COMPILER_ROOT

if [[ ! -f "$CROSS_BINPACKET.tar.gz" ]]; then
	echo "No BINUTILS package found, download from $CROSS_RESOURCE..."
	echo "Downloading BINUTILS package from $CROSS_BINREMOTE..."
	wget $CROSS_BINREMOTE
fi

tar -C $CROSS_COMPILER_TARGET -xvzf "$CROSS_BINPACKET.tar.gz"

if [[ ! -f "$CROSS_GCCPACKET.tar.gz" ]]; then
	echo "No GCC package found, download from $CROSS_RESOURCE..."
	echo "Downloading GCC package from $CROSS_GCCREMOTE..."
	wget $CROSS_GCCREMOTE
fi

tar -C $CROSS_COMPILER_TARGET -xvzf "$CROSS_GCCPACKET.tar.gz"

cd $CROSS_COMPILER_PREFIX

mkdir -p "$CROSS_COMPILER_TARGET-binutils"
cd "$CROSS_COMPILER_TARGET-binutils"
../$CROSS_BINPACKET/configure 			\
	--target=$CROSS_COMPILER_TARGET		\
	--prefix=$CROSS_COMPILER_PREFIX		\
	--with-sysroot				\
	--disable-nls				\
	--disable-werror
make
make install

cd $CROSS_COMPILER_PREFIX

mkdir -p "$CROSS_COMPILER_TARGET-gcc"
cd "$CROSS_COMPILER_TARGET-gcc"
../$CROSS_GCCPACKET/configure 			\
	--target=$CROSS_COMPILER_TARGET		\
	--prefix=$CROSS_COMPILER_PREFIX		\
	--disable-nls				\
	--enable-languages=c,c++		\
	--without-headers

make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

exit 0