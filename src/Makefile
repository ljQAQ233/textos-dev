SHELL := bash
ARCH ?= x86_64

export ARCH

# toolchain
HOSTCC ?= gcc
HOSTLD ?= ld
HOSTOC ?= objcopy
CROSS_COMPILE ?= ${ARCH}-elf-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

export CROSS_COMPILE
export HOSTCC HOSTLD HOSTOC
export CC LD OBJCOPY

# available modes:
#   efi - uefi / ovmf
#   emulti - qemu multiboot
BOOT_MODE ?= efi

# === config ends here!!! ===

export BOOT_MODE

include config/main.mk
include config/image.mk

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
include config/net.mk
include config/udk.mk
include config/qemu.mk

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

clean:
	make -C app clean
	make -C boot clean
	make -C kernel clean
	find . -iname "*.status_log" | xargs rm -f
	find . -iname "*.status_lock" | xargs rm -f
	rm -rf $(OUTPUT)

.PHONY: all image qemu qemug qemubg \
		clean compile_commands.json $(MODS)

# 子模块路由：include config/subdir.mk 提供 @ 语法
#   make -C src app/lua@test      → 递归到 app/lua 执行 test
#   make -C src @compile_commands → 当前层执行 compile_commands
$(MODS):
	$(MAKE) -C $@

_SUBDIR_PRE  := $(PRETARGETS)
include config/subdir.mk
