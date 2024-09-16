ARCH ?= X64

# Commands
CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

# Include path
INCLUDE := $(SRC_DIR)/include

CFLAGS  := -static
CFLAGS  += -nostdlib
CFLAGS  += -nostdinc
CFLAGS  += -fshort-wchar
CFLAGS  += -fno-builtin
CFLAGS  += -ffreestanding
CFLAGS  += -fno-stack-protector
CFLAGS  += $(addprefix -I,${INCLUDE})
CFLAGS  += -g -O0
CFLAGS  += -std=c11

