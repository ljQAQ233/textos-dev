QEMU_BINARY = /usr/bin
# Set Qemu Executable File Path

QEMU = $(QEMU_BINARY)/qemu-system-x86_64

OVMF        = $(BASE)/OVMF_RELEASE_$(ARCH).fd
OVMF_DEBUG  = $(BASE)/OVMF_DEBUG_$(ARCH).fd
OVMF_NOOPT  = $(BASE)/OVMF_NOOPT_$(ARCH).fd
# the BIOS floppy for Qemu to Run

MEM = 64M

QEMU_FLAGS := -hda $(IMG_OUTPUT) \
			   -net none \
			   -m $(MEM) \
			   -no-reboot

# Qemu Common Args

QEMU_FLAGS_RUN  := $(QEMU_FLAGS) \
			   -bios $(OVMF)

QEMU_FLAGS_DBG  := $(QEMU_FLAGS) \
#			   -d int,cpu_reset


# No graphic for debugging
ifeq (${QEMU_GPY},false)
  QEMU_FLAGS_DBG += -nographic
endif

QEMU_FLAGS_BDBG  := $(QEMU_FLAGS_DBG) \
			   -bios $(OVMF_NOOPT) \
			   -serial pipe:$(PIPE)

# Qemu Args for Debugging
