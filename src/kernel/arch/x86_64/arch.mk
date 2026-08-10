SRCS += $(ARCH_DIR)/boot/bootpgt.S
SRCS += $(ARCH_DIR)/boot/common.c
SRCS += $(ARCH_DIR)/boot/efi.c
SRCS += $(ARCH_DIR)/boot/efi.S
SRCS += $(ARCH_DIR)/boot/multi.c
SRCS += $(ARCH_DIR)/boot/multi.S
SRCS += $(ARCH_DIR)/boot/_start.S

SRCS += $(ARCH_DIR)/cpu.S
SRCS += $(ARCH_DIR)/ap.c
SRCS += $(ARCH_DIR)/ap.S
SRCS += $(ARCH_DIR)/mycpu.c
SRCS += $(ARCH_DIR)/io.S
SRCS += $(ARCH_DIR)/gdt.c
SRCS += $(ARCH_DIR)/intr.c
SRCS += $(ARCH_DIR)/intr.S
SRCS += $(ARCH_DIR)/irq.S
SRCS += $(ARCH_DIR)/time.c
SRCS += $(ARCH_DIR)/ptrace.c

$(BUILD)/$(ARCH_DIR)/boot/multi.c.o: CFLAGS += -fno-asynchronous-unwind-tables
$(BUILD)/$(ARCH_DIR)/boot/common.c.o: CFLAGS += -fno-asynchronous-unwind-tables
$(BUILD)/$(ARCH_DIR)/boot/efi.c.o: CFLAGS += -fno-asynchronous-unwind-tables
