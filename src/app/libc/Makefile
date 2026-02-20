SRCS := $(wildcard *.c */*.c)
SRCS := $(filter-out linker/%,$(SRCS))
SRCS := $(filter-out crt1.c crtn.c crti.c, $(SRCS))

BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${BUILD}/,${OBJS})
CRTO := \
	$(BUILD)/crt1.c.o \
	$(BUILD)/crti.c.o \
	$(BUILD)/crtn.c.o

CFLAGS += -nostdinc -nostdlib -Wl,-z,noexecstack

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
	@ar rcs $@ $^
	@echo -e "\033[032m   AR  \033[0m $@"

$(BUILD)/%.so: $(BUILD)/%.o
	@$(LD) $(LDFLAGS) $^ -o $@ -shared -fPIC
	@echo -e "\033[032m   CC  \033[0m $@"

$(BUILD)/libc.o: $(OBJS)
	@$(LD) $(LDFLAGS) $^ -r -o $@
	@echo -e "\033[032m   LD  \033[0m link -> $@"

$(BUILD)/libdl.o: linker/libdl.c linker/linker.c
	@$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	@echo -e "\033[032m   CC  \033[0m $<"

$(BUILD)/ld-textos.so: linker/ldso.c linker/linker.c $(CRTO)
	@$(CC) $(CFLAGS) $< $(CRTO) -o $@ -static -L$(BUILD) -lc \
		-ffunction-sections -fdata-sections -Wl,--gc-sections
	@echo -e "\033[032m   CC  \033[0m ldso -> $@"

$(BUILD)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: all
