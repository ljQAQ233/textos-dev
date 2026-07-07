OUTPUT := ../build
OUTPUT := $(abspath ${OUTPUT})


# 转化为 edk2 配置
efi_arch_x86 = IA32
efi_arch_x86_64 = X64
EFI_ARCH := $(efi_arch_$(ARCH))

export EFI_ARCH

ifneq ($(shell git rev-parse --is-inside-work-tree 2>/dev/null),true)
  GIT_HASH := unknown
else
  GIT_HASH := $(shell git rev-parse --short HEAD)
endif

export GIT_HASH

SRC_DIR := $(shell pwd)
BASE  := $(SRC_DIR)/base
UTILS := $(SRC_DIR)/utils

export SRC_DIR BASE UTILS

# with the macro 'OUTPUT' in Boot.dsc, indicating the output directory
BOOT_OUTPUT := $(OUTPUT)/boot

ifeq (${ARCH},x86_64)
  BOOT_EXEC := $(BOOT_OUTPUT)/BootX64.efi
else
  $(error unsupported arch : $(ARCH))
  BOOT_EXEC := $(BOOT_OUTPUT)/BootIa32.efi
endif

KERNEL_OUTPUT := $(OUTPUT)/kernel
KERNEL_EXEC   := $(KERNEL_OUTPUT)/kernel.elf

APP_OUTPUT    := $(OUTPUT)/app

export BOOT_OUTPUT BOOT_EXEC KERNEL_OUTPUT KERNEL_EXEC APP_OUTPUT

# makefile will deal with x.h.in
%.h: %.h.in
	sed -f $(UTILS)/mkalltypes.sed $< > $@
