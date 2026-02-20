# Set qemu executable file path
QEMU_HOME ?= /usr
QEMU_BIN := $(QEMU_HOME)/bin
QEMU := $(QEMU_BIN)/qemu-system-x86_64

QEMU_LOG ?= file:$(OUTPUT)/qemu.log
QEMU_MEM ?= 64M

# Qemu Common Args
QEMU_FLAGS := \
  -cpu qemu64,+x2apic \
  -smp 2 -m $(QEMU_MEM) \
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
ifeq (${BOOT_MODE},emulti)
  define fw_arg
    -kernel $(KERNEL_EXEC)
  endef
else
  define fw_arg
    -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(EFI_ARCH).code,readonly=on \
    -drive if=pflash,format=raw,file=$(BASE)/OVMF_$(1)_$(EFI_ARCH).vars
  endef
endif

ifeq (${QEMU_AHCI},true)
  QEMU_FLAGS += \
	-device ahci,id=ahci0 \
	-drive file=$(IMG),if=none,id=sata1 \
    -device ide-hd,drive=sata1,bus=ahci0.0
else
  QEMU_FLAGS += -drive file=$(IMG),if=ide,index=0,media=disk
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
