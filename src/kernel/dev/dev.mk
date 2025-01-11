
SRCS += dev/8259.c
SRCS += dev/pit.c
SRCS += dev/serial.c
SRCS += dev/keyboard.c

SRCS += dev/clock.c

# disk drivers

SRCS += dev/disk/buffer.c
SRCS += dev/disk/ide.c

# fpu support

SRCS += dev/fpu/fpu.c
SRCS += dev/fpu/i387.s

# pci devices
SRCS += dev/pci.c

# pseudo-dev
SRCS += dev/mem.c
