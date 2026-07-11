TEST_SRCS := \
  $(wildcard sys_*.c) \
  $(wildcard libc_*.c)

SRCS := $(TEST_SRCS)
SRCS += runtest.c
SRCS += unity/unity.c

TARG := runtest

GENS := $(wildcard *.py)
GTARS := \
  $(GENS:%.py=auto/%.h) \
  auto/unity.h

# external binaries
BINS := $(wildcard bin/*.c)

# dynamic libraries for test
SOLIBS := $(wildcard solib/*.c)

CC := $(BUILD)/libc/cc
CFLAGS += -g

include ../all.mk

$(OBJS): CFLAGS += -I$(CURDIR)/unity
$(OBJS): CFLAGS += -DUNITY_INCLUDE_CONFIG_H
$(TARG): LDLIBS += -lm -ldl
$(TARG): LDFLAGS += -static

BTARS := $(BINS:%.c=${BUILD}/%)
SOTARS := $(SOLIBS:%.c=${BUILD}/%.so)

all: $(GTARS) $(TARG) $(BTARS) $(SOTARS)

auto/%.h: %.py
	@mkdir -p $(@D)
	@python $< > $@
	@echo -e "\033[032m   PY  \033[0m $<"

# #define TESTS \
# _(function, name, line)
auto/unity.h: $(TEST_SRCS)
	@mkdir -p $(@D)
	@printf '#define TESTS' > $@
	@grep '// *! *register=' $^ -Hn | \
		sed 's/\/\/ *! *register *= *//' | \
		awk -F: '{printf " \\\n_(\"%s\",%s,\"%s\",%d)",$$1,$$3,$$3,$$2}' >> $@
	@printf '\n' >> $@
	@echo -e "\033[032m   OK  \033[0m $@"

$(BUILD)/bin/%: bin/%.c
	$(call compile-cc,$<,$@)

$(BUILD)/solib/%.so: solib/%.c
	$(call compile-cc,$<,$@,-shared -fPIC)

$(BUILD)/bin/dlopen: LDLIBS += -ldl
$(BUILD)/solib/add.so: $(BUILD)/solib/dep.so
$(BUILD)/solib/add.so: LDFLAGS += -nostdlib
$(BUILD)/solib/dep.so: LDFLAGS += -nostdlib

.DEFAULT_GOAL := all
.PHONY: all

include ../../make/install-leaf.mk
