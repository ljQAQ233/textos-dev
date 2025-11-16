# Set Qemu Executable File Path
QEMU_BINARY = /usr/bin

QEMU = $(QEMU_BINARY)/qemu-system-x86_64

QEMU_LOG ?= file:$(OUTPUT)/qemu.log

MEM = 64M

# Qemu Common Args
QEMU_FLAGS := \
  -hda $(IMG) \
  -cpu qemu64,+x2apic \
  -smp 2 -m $(MEM) \
  -no-reboot \
  -debugcon $(QEMU_LOG) \
  -device isa-debug-exit \
  -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
  -device e1000,netdev=net0,mac=52:54:00:12:34:56

# No graphic mode
# TODO: make udk debugger use tcp to connect to QEMU rather than serial and named pipe
ifeq (${QEMU_GPY},false)
  ifneq (${BSRC_DEBUG},true)
    QEMU_FLAGS += -nographic
  endif
endif

# Firmware selection
ifeq (${QEMU_EFI},false)
  define fw_arg
    -kernel $(KERNEL_EXEC)
  endef
else
  define fw_arg
    -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(ARCH).code,readonly=on \
    -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(ARCH).vars
  endef
endif

QEMU_FLAGS_RUN := \
  $(QEMU_FLAGS) \
  $(call fw_arg,RELEASE)

QEMU_FLAGS_DBG := $(QEMU_FLAGS)

QEMU_FLAGS_BDBG := \
  $(QEMU_FLAGS_DBG) \
  $(call fw_arg,NOOPT)

QEMU_FLAGS_KDBG := \
  $(QEMU_FLAGS_DBG) \
  -s -S \
  $(call fw_arg,RELEASE)
