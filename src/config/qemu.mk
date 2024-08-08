QEMU_BINARY = /usr/bin
# Set Qemu Executable File Path

QEMU = $(QEMU_BINARY)/qemu-system-x86_64

OVMF        = $(BASE)/OVMF_RELEASE_$(ARCH).fd
OVMF_DEBUG  = $(BASE)/OVMF_DEBUG_$(ARCH).fd
OVMF_NOOPT  = $(BASE)/OVMF_NOOPT_$(ARCH).fd
# the BIOS floppy for Qemu to Run

ifdef QEMU_LOG
  QEMU_SERIAL = file:$(OUTPUT)/qemu.srl
else
  QEMU_SERIAL = stdio
endif

QEMU_LOG ?= file:$(OUTPUT)/qemu.log

MEM = 64M

QEMU_FLAGS := -hda $(IMG_OUTPUT) \
			   -net none \
			   -cpu qemu64,+x2apic \
			   -m $(MEM) \
			   -no-reboot \
			   -debugcon $(QEMU_LOG)

# Qemu Common Args

QEMU_FLAGS_RUN  := $(QEMU_FLAGS) \
			   -bios $(OVMF)

QEMU_FLAGS_DBG  := $(QEMU_FLAGS) \
#			   -d int,cpu_reset

# No graphic for debugging
ifeq (${QEMU_GPY},false)
  ifeq (${BSRC_DEBUG},false)
    QEMU_FLAGS_DBG += -nographic
  endif
endif

QEMU_FLAGS_BDBG  := $(QEMU_FLAGS_DBG) \
			   -bios $(OVMF_NOOPT)

QEMU_FLAGS_KDBG  := $(QEMU_FLAGS_DBG) \
			   -bios $(OVMF) \
			   -s -S \
			   -serial $(QEMU_SERIAL)

# Qemu Args for Debugging
