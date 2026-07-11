SHELL := bash
ARCH ?= x86_64
CROSS_COMPILE ?= ${ARCH}-elf-

# toolchain
HOSTCC ?= gcc
HOSTLD ?= ld
HOSTOC ?= objcopy

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

INSTALL_SUDO =
INSTALL = $(INSTALL_SUDO) install

# available modes:
#   efi - uefi / ovmf
#   emulti - qemu multiboot
BOOT_MODE ?= efi

TOPBUILD := ../build
TOPBUILD := $(abspath $(TOPBUILD))
BUILD := $(TOPBUILD)

# === config ends here!!! ===

export ARCH CROSS_COMPILE
export HOSTCC HOSTLD HOSTOC
export CC LD OBJCOPY
export INSTALL
export BOOT_MODE
export TOPBUILD BUILD

SRC_DIR := $(shell pwd)
BASE  := $(SRC_DIR)/base
UTILS := $(SRC_DIR)/utils

export SRC_DIR BASE UTILS

include make/xcommon.mk
include make/ximage.mk

all: boot kernel app image

MODS := kernel boot app
PRETARGETS := $(ROOT) include/bits/alltypes.h
$(MODS): $(PRETARGETS)

# ovmf
ovmf:
	@make -C boot TARGET=RELEASE ovmf

ovmf-debug:
	@make -C boot TARGET=DEBUG FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE" ovmf

ovmf-noopt:
	@make -C boot TARGET=NOOPT FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE -DSOURCE_DEBUG_ENABLE=TRUE" ovmf

# image maker
image: $(IMG)

# QEMU commands
#   - qemu - run
#   - qemug - kernel debug
#   - qemubg - uefi debug (udk)
include make/xnet.mk
include make/xudk.mk
include make/xqemu.mk

qemu: all net
	$(QEMU) $(QEMU_FLAGS_RUN)

qemug: all net
	$(QEMU) $(QEMU_FLAGS_KDBG)

qemubg: export BSRC_DEBUG=true
qemubg: export PIPE=$(shell cat $(UDK_DBG_CFG) | grep -E "^[P ]ort" | awk '{print $$NF}')
qemubg: all net
	mkfifo $(PIPE).in $(PIPE).out
	$(UDK_DBG_EXEC) 2>&1 &
	$utils/monitor_qemu.sh > /dev/null 2>&1 & disown -h
	$(QEMU) $(QEMU_FLAGS_BDBG) -serial pipe:$(PIPE)

# toolkits
compile_commands.json: clean
	@bear --output ../compile_commands.json -- make all
	@utils/fix_clcmd.sh ../compile_commands.json

include/bits/alltypes.h: include/bits/alltypes.h.in
	sed -f $(UTILS)/mkalltypes.sed $< > $@

clean:
	make -C app clean
	make -C boot clean
	make -C kernel clean
	find . -iname "*.status_log" | xargs rm -f
	find . -iname "*.status_lock" | xargs rm -f
	-rm -rf $(TOPBUILD)
	-sudo rm -rf $(TOPBUILD)

.PHONY: all image qemu qemug qemubg \
		clean compile_commands.json $(MODS)

$(MODS):
	$(MAKE) -C $@

_SUBDIR_PRE  := $(PRETARGETS)
include make/subdir.mk
include make/install-node.mk
