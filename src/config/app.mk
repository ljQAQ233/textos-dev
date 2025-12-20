ARCH ?= X64

# Commands
CC      := gcc
LD      := ld
OBJCOPY := objcopy

# Include path
INCLUDE := \
  $(SRC_DIR)/include \
  $(SRC_DIR)/include/arch/$(ARCH) \
  $(SRC_DIR)/app/lvgl \
  $(APP_OUTPUT)/libm/include/openlibm

LIBRARY := \
  $(APP_OUTPUT)/libc \
  $(APP_OUTPUT)/libm

CFLAGS := -g \
  -std=c11 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -include $(SRC_DIR)/include/bits/compiler.h \
  $(addprefix -I,${INCLUDE})

LDFLAGS := \
  -nostdlib

export CC LD OBJCOPY INCLUDE LIBRARY CFLAGS LDFLAGS
