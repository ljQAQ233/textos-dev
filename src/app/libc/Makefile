SRCS := $(wildcard *.c */*.c)
SRCS := $(filter-out linker/%,$(SRCS))
SRCS := $(filter-out crt1.c crtn.c crti.c, $(SRCS))

include $(dir $(lastword $(MAKEFILE_LIST)))/../cmd.mk

BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${BUILD}/,${OBJS})
CRTO := \
	$(BUILD)/crt1.c.o \
	$(BUILD)/crti.c.o \
	$(BUILD)/crtn.c.o

CC := $(ORIG_CC)
CFLAGS += -nostdinc -nostdlib -Wl,-z,noexecstack
CFLAGS += -std=c11 -fshort-wchar -ffreestanding \
  -fno-builtin -fno-stack-check -fno-stack-protector \
  -include $(SRC_DIR)/include/bits/compiler.h
LDFLAGS += -nostdlib

all: $(BUILD)/cc \
	$(CRTO) \
	$(BUILD)/libc.a \
	$(BUILD)/libc.so \
	$(BUILD)/libdl.a \
	$(BUILD)/libdl.so \
	$(BUILD)/ld-textos.so
	@mkdir -p $(ROOT)/lib
	@cp $(filter %.so,$^) $(ROOT)/lib

$(BUILD)/cc: \
	$(BUILD)/cc.specs
	@echo '#!/bin/sh' > $@
	@echo '$(CC) "$$@" -specs=$<' >> $@
	@chmod +x $@

$(BUILD)/cc.specs:
	@mkdir -p $(@D)
	@$(CURDIR)/specs.sh "$(INCLUDE)" "$(BUILD)" "$(LIBRARY)" "/lib/ld-textos.so" > $@
	# @sudo ln -sf $(BUILD)/ld-textos.so /lib/ld-textos.so
	@touch .stamp

$(BUILD)/%.a: $(BUILD)/%.o
	$(call compile-ar,$^,$@)

$(BUILD)/%.so: $(BUILD)/%.o
	$(call compile-so,$^,$@)

$(BUILD)/libc.o: $(OBJS)
	$(call compile-merge,$^,$@)

$(BUILD)/libdl.o: linker/libdl.c linker/linker.c
	$(call compile-obj,$<,$@,-fPIC)

$(BUILD)/ld-textos.so: linker/ldso.c linker/linker.c $(CRTO)
	@$(CC) $(CFLAGS) $< $(CRTO) -o $@ -static -L$(BUILD) -lc \
		-ffunction-sections -fdata-sections -Wl,--gc-sections
	@echo -e "\033[032m   CC  \033[0m ldso -> $@"

$(BUILD)/%.c.o: %.c
	$(call compile-obj,$<,$@,-fPIC)

.PHONY: all
