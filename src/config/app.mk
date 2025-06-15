ARCH ?= X64

# Commands
CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

# Include path
INCLUDE := \
  $(SRC_DIR)/include \
  $(SRC_DIR)/include/arch/$(ARCH) \
  $(SRC_DIR)/app/lvgl

LIBC := $(APP_OUTPUT)/libc/libc.o

CFLAGS := \
  -static -nostdlib -nostdinc \
  -std=c11 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -include $(SRC_DIR)/include/bits/compiler.h \
  $(addprefix -I,${INCLUDE}) \

LDFLAGS := \
  -static -nostdlib \

export CC LD NASM OBJCOPY LIBC CFLAGS LDFLAGS

