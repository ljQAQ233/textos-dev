SHELL  := bash
OUTPUT := $(KERNEL_OUTPUT)

export OUTPUT ARCH_DIR

# Include path
INCLUDE := \
  $(SRC_DIR)/include \
  $(SRC_DIR)/include/textos/klib \
  $(SRC_DIR)/include/arch/$(ARCH) \
  $(abspath arch/$(ARCH))

CFLAGS := \
  -static -nostdlib -nostdinc -g -Wall \
  -std=c11 -O0 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -mcmodel=large -mno-red-zone \
  -include $(SRC_DIR)/include/textos/textos.h \
  -include $(KERNEL_OUTPUT)/autogen.h \
  $(addprefix -I,${INCLUDE})

CFLAGS += -D__TEXTOS_DEBUG

AFLAGS := \
  -g

LDFLAGS := \
  -static -nostdlib -z noexecstack \
  -no-relax -z max-page-size=0x1000 \
  -Map=$(KERNEL_OUTPUT)/system.map

ARCH_DIR = arch/$(ARCH)

SRCS := \
  init.c main.c debug.c acpi.c apic.c dev.c \
  printk.c ktm.c ktimer.c time.c task.c task.S signal.c ptrace.c file.c \
  mycpu.c pwm.c lock.c noopt.c

include klib/klib.mk
include mm/mm.mk
include video/video.mk
include lai/lai.mk
include dev/dev.mk
include fs/fs.mk
include net/net.mk
include user/user.mk

include $(ARCH_DIR)/arch.mk

OBJS := $(SRCS:.c=.c.o)
OBJS := $(OBJS:.S=.S.o)
OBJS := $(addprefix ${OUTPUT}/,${OBJS})

kernel: $(KERNEL_OUTPUT) autogen $(KERNEL_EXEC)
	@cp $(KERNEL_EXEC) $(ROOT)

autogen:
	@printf '%s\n' > $(KERNEL_OUTPUT)/autogen.h \
		'#ifndef __AUTOGEN_H__'                 \
		'#define __AUTOGEN_H__'                 \
		''                                      \
		'#define UTS_RELEASE    "$(GIT_HASH)"'  \
		'#define UTS_VERSION    "'"`date`"'"'   \
		''                                      \
		'#endif'


$(KERNEL_EXEC): $(OBJS)
	@$(LD) $(LDFLAGS) $^ -T <($(CPP) -P linker.ld.S) -o $@
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

$(OUTPUT)/%.S.o: %.S
	@mkdir -p $(@D)
	@$(CC) $(AFLAGS) -c $< -o $@
	@echo -e "\033[032m   ASM \033[0m $<"

$(OUTPUT)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: kernel autogen clean
