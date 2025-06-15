SRC_DIR  := $(shell pwd)
# Set Source Root

ifneq ($(shell git rev-parse --is-inside-work-tree 2>/dev/null),true)
  GIT_HASH := unknown
else
  GIT_HASH := $(shell git rev-parse --short HEAD)
endif

OUTPUT   := ../build
# avoid location error
OUTPUT   := $(abspath ${OUTPUT})

# with the macro 'OUTPUT' in Boot.dsc, indicating the output directory
BOOT_OUTPUT := $(OUTPUT)/boot

ifeq (${ARCH},X64)
  BOOT_EXEC := $(BOOT_OUTPUT)/BootX64.efi
else
  $(error unsupported arch : $(ARCH))
  BOOT_EXEC := $(BOOT_OUTPUT)/BootIa32.efi
endif

KERNEL_OUTPUT := $(OUTPUT)/kernel
KERNEL_EXEC   := $(KERNEL_OUTPUT)/kernel.elf

APP_OUTPUT    := $(OUTPUT)/app

SHELL  := bash

BASE   := base
UTILS  := utils
BASE   := $(abspath ${BASE})
UTILS  := $(abspath ${UTILS})

# makefile will deal with x.h.in
%.h: %.h.in
	sed -f $(UTILS)/mkalltypes.sed $< > $@

export SHELL
export GIT_HASH
export SRC_DIR BASE UTILS
export BOOT_OUTPUT BOOT_EXEC KERNEL_OUTPUT KERNEL_EXEC APP_OUTPUT

