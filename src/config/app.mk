ARCH ?= X64

# Commands
CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

# Include path
INCLUDE := \
  $(SRC_DIR)/include \
  $(SRC_DIR)/include/app \
  $(SRC_DIR)/include/arch/$(ARCH)

CFLAGS := \
  -static -nostdlib -g \
  -std=c11 -O0 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -include $(SRC_DIR)/include/app/app.h \
  $(addprefix -I,${INCLUDE}) \

LDFLAGS := \
  -static -nostdlib \

LIBC := $(APP_OUTPUT)/libc.o
