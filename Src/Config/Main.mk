SRC_DIR  := $(shell pwd)
# Set Source Root

OUTPUT   := ../Build
OUTPUT   := $(abspath ${OUTPUT})
# Get the RealPath

BOOT_OUTPUT     := $(OUTPUT)/Boot
# For a Macro in .dsc file I defined,the output directory
ifeq (${ARCH},X64)
  BOOT_EXEC     := $(BOOT_OUTPUT)/BootX64.efi
else
  BOOT_EXEC     := $(BOOT_OUTPUT)/BootIa32.efi
endif
# Boot Built Executable EFI File Output File on Host

ifeq (${ARCH},X64)
  IMG_EFI = $(IMG_MDIR)/EFI/Boot/bootX64.efi
else
  $(error [ERR] Unsupported arch : $(ARCH))
  # IMG_EFI = $(IMG_MDIR)/EFI/Boot/bootia32.efi
  # QEMU    = $(QEMU_BINARY)/qemu-system-i386
endif

SHELL  := bash
SUDO   := echo q | sudo -S
# including password !!!

BASE   := Base
UTILS  := Utils
BASE   := $(abspath ${BASE})
UTILS  := $(abspath ${UTILS})

export CC NASM SHELL TERM SUDO
export SRC_DIR BASE UTILS
export BOOT_OUTPUT BOOT_EXEC IMG_EFI
