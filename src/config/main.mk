SRC_DIR  := $(shell pwd)
# Set Source Root

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

SHELL  := bash

BASE   := base
UTILS  := utils
BASE   := $(abspath ${BASE})
UTILS  := $(abspath ${UTILS})

export SHELL
export SRC_DIR BASE UTILS
export BOOT_OUTPUT BOOT_EXEC
