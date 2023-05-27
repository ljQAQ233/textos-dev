ARCH    ?= X64

# Commands
CC      := gcc
LD      := ld
NASM    := nasm

# Include path
INCLUDE := \
  $(SRC_DIR)/include \
  $(abspath arch/$(ARCH))

CFLAGS := \
  -static -nostdlib -nostdinc \
  -std=c11 -O0 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -include $(SRC_DIR)/include/textos/textos.h \
  $(addprefix -I,${INCLUDE})

# Nasm flags
NFLAGS := \
  -f elf64

LDFLAGS := \
  -static -nostdlib \
  -z max-page-size=0x1000 \
  -Map=$(KERNEL_OUTPUT)/system.map

