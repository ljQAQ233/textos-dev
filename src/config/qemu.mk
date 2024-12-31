# Set Qemu Executable File Path
QEMU_BINARY = /usr/bin

QEMU = $(QEMU_BINARY)/qemu-system-x86_64

ifdef QEMU_LOG
  QEMU_SERIAL = file:$(OUTPUT)/qemu.srl
else
  QEMU_SERIAL = stdio
endif

QEMU_LOG ?= file:$(OUTPUT)/qemu.log

MEM = 64M

# Qemu Common Args
QEMU_FLAGS := -hda $(IMG) \
			   -net none \
			   -cpu qemu64,+x2apic \
			   -m $(MEM) \
			   -no-reboot \
			   -debugcon $(QEMU_LOG) \
			   -device isa-debug-exit

# Firmware selection
define fw_arg
  -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(ARCH).code,readonly=on \
  -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(ARCH).vars
endef

QEMU_FLAGS_RUN  := $(QEMU_FLAGS) \
				   $(call fw_arg,RELEASE)

QEMU_FLAGS_DBG  := $(QEMU_FLAGS)

# No graphic for debugging
ifeq (${QEMU_GPY},false)
  ifeq (${BSRC_DEBUG},false)
    QEMU_FLAGS_DBG += -nographic
  endif
endif

QEMU_FLAGS_BDBG := $(QEMU_FLAGS_DBG) \
				   $(call fw_arg,NOOPT)

QEMU_FLAGS_KDBG := $(QEMU_FLAGS_DBG) \
				   -s -S \
				   -serial $(QEMU_SERIAL) \
				   $(call fw_arg,RELEASE) \
				   # -bios base/OVMF_RELEASE_X64.fd

# Qemu Args for Debugging
