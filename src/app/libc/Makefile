BUILD := $(BUILD)/$(notdir ${CURDIR})

SRCS := $(wildcard *.c */*.c)
SRCS := $(filter-out linker/%,$(SRCS))
SRCS := $(filter-out crt1.c crtn.c crti.c, $(SRCS))

include $(dir $(lastword $(MAKEFILE_LIST)))/../cmd.mk

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

# since we don't have openlibm compiled before libc's building
# add original source code path here to support fenv.h access
CFLAGS += -I $(SRC_DIR)/app/libm/openlibm/src
CFLAGS += -I $(SRC_DIR)/app/libm/openlibm/include

all: $(BUILD)/cc \
	$(CRTO) \
	$(BUILD)/libc.a \
	$(BUILD)/libc.so \
	$(BUILD)/libdl.a \
	$(BUILD)/libdl.so \
	$(BUILD)/ld-textos.so

$(BUILD)/cc: \
	$(BUILD)/cc.specs
	@echo '#!/bin/sh' > $@
	@echo "$(CC)"
	@echo "$(ORIG_CC)"
	@echo '$(CC) "$$@" -specs=$<' >> $@
	@chmod +x $@

$(BUILD)/cc.specs:
	@mkdir -p $(@D)
	@$(CURDIR)/specs.sh "$(INCLUDE)" "$(BUILD)" "$(LIBRARY)" "/lib/ld-textos.so" > $@
	# @sudo ln -sf $(BUILD)/ld-textos.so /lib/ld-textos.so

$(BUILD)/%.a: $(BUILD)/%.o
	$(call compile-ar,$^,$@)

$(BUILD)/%.so: $(BUILD)/%.o
	$(call compile-so,$^,$@)

# link libgcc directly
$(BUILD)/libc.o: $(OBJS)
	$(call compile-merge,$^,$@,-lgcc)
	@touch .stamp

$(BUILD)/libdl.o: linker/libdl.c linker/linker.c
	$(call compile-obj,$<,$@,-fPIC)

$(BUILD)/ld-textos.so: linker/ldso.c linker/linker.c $(CRTO)
	@$(CC) $(CFLAGS) $< $(CRTO) -o $@ -static -L$(BUILD) -lc \
		-ffunction-sections -fdata-sections -Wl,--gc-sections
	@echo -e "\033[032m   CC  \033[0m ldso -> $@"

$(BUILD)/%.c.o: %.c
	$(call compile-obj,$<,$@,-fPIC)

.PHONY: all
