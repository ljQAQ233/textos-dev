SHELL  := bash
OUTPUT := $(KERNEL_OUTPUT)

ARCH_DIR = arch/$(ARCH)

export OUTPUT ARCH_DIR

SRCS := init.c main.c \
		debug.c assert.c panic.c \
		acpi.c apic.c dev.c \
		console.c \
		printk.c \
		task.c task.s file.c \
		pwm.c

include klib/klib.mk
include mm/mm.mk
include video/video.mk
include dev/dev.mk
include fs/fs.mk
include user/user.mk

include $(ARCH_DIR)/arch.mk

OBJS := $(SRCS:.c=.c.o)
OBJS := $(OBJS:.s=.s.o)
OBJS := $(OBJS:.asm=.asm.o)

OBJS := $(addprefix ${OUTPUT}/,${OBJS})

include ../config/kernel.mk

kernel: $(KERNEL_OUTPUT) $(KERNEL_EXEC)

$(KERNEL_EXEC): $(OBJS)
	@$(LD) $(LDFLAGS) $^ -T linker.ld -o $@
	@echo -e "\033[032m   LD  \033[0m Linked all objects -> $@"
	@cp $@ $@.symbol
	@echo -e "\033[032m   OCP \033[0m $@ -> ${@:$(OUTPUT)/%=%}.symbol"
	@$(OBJCOPY) --strip-debug $@ $@

$(KERNEL_OUTPUT):
	@mkdir -p $(KERNEL_OUTPUT)
	@echo -e "\033[032m   MKD \033[0m Created directory $@"

clean:
	find . -iname "*.s.o"   | xargs rm -f
	find . -iname "*.asm.o" | xargs rm -f
	find . -iname "*.c.o"   | xargs rm -f

# Make rules and printing formats

$(OUTPUT)/%.s.o: %.s
	@mkdir -p $(@D)
	@$(NASM) $(NFLAGS) $< -o $@
	@echo -e "\033[032m   ASM \033[0m $<"

$(OUTPUT)/%.asm.o: %.asm
	@mkdir -p $(@D)
	@$(NASM) $(NFLAGS) $< -o $@
	@echo -e "\033[032m   ASM \033[0m $<"

$(OUTPUT)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: kernel clean
