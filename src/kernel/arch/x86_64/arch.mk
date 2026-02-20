
SRCS += $(ARCH_DIR)/multi.S
SRCS += $(ARCH_DIR)/multi.c
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

$(OUTPUT)/$(ARCH_DIR)/multi.c.o: CFLAGS += -fno-asynchronous-unwind-tables
