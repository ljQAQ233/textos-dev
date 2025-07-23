
SRCS += dev/8259.c
SRCS += dev/hpet.c
SRCS += dev/pit.c
SRCS += dev/serial.c
SRCS += dev/dbgcon.c
SRCS += dev/fbdev.c
SRCS += dev/keyboard.c

# tty or pts
SRCS += dev/tty/tty.c
SRCS += dev/tty/tty_buffer.c


SRCS += dev/clock.c

# disk drivers

SRCS += dev/disk/buffer.c
SRCS += dev/disk/part.c
SRCS += dev/disk/ide.c

# fpu support

SRCS += dev/fpu/fpu.c
SRCS += dev/fpu/i387.s

# pci devices
SRCS += dev/pci.c

# network
SRCS += dev/nic/nic.c
SRCS += dev/nic/mbuf.c
SRCS += dev/nic/e1000.c

# pseudo-dev
SRCS += dev/mem.c
SRCS += dev/anony.c
