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

CC := $(APP_OUTPUT)/libc/cc
LDFLAGS += -static

include ../all.mk

$(OBJS): CFLAGS := -I$(CURDIR)/unity
$(OBJS): CFLAGS += -DUNITY_INCLUDE_CONFIG_H
$(TARG): LDLIBS += -lm

BTARS := $(BINS:%.c=${APP_OUTPUT}/%)
SOTARS := $(SOLIBS:%.c=${APP_OUTPUT}/%.so)

all: $(GTARS) $(TARG) $(BTARS) $(SOTARS)
	@mkdir -p $(ROOT)/test
	@mkdir -p $(ROOT)/test/bin
	@mkdir -p $(ROOT)/test/solib
	@cp $(TARG) $(ROOT)/test
	@for f in $(BTARS); do cp $$f $(ROOT)/test/bin/$$(basename $$f); done
	@for f in $(SOTARS); do cp $$f $(ROOT)/test/solib/$$(basename $$f); done

auto/%.h: %.py
	@mkdir -p $(@D)
	@python $< > $@
	@echo -e "\033[032m   PY  \033[0m $<"

# #define TESTS \
# _(function, name, line)
auto/unity.h: $(TEST_SRCS)
	@mkdir -p $(@D)
	@printf '#define TESTS' > $@
	@grep '//!register=' $^ -Hn | \
		sed 's/\/\/!register=//' | \
		awk -F: '{printf " \\\n_(\"%s\",%s,\"%s\",%d)",$$1,$$3,$$3,$$2}' >> $@
	@printf '\n' >> $@
	@echo -e "\033[032m   OK  \033[0m $@"

$(APP_OUTPUT)/bin/%: bin/%.c
	$(call compile-cc,$<,$@)

$(APP_OUTPUT)/solib/%.so: solib/%.c
	$(call compile-cc,$<,$@,-shared -fPIC)

$(APP_OUTPUT)/bin/dlopen: LDLIBS += -ldl

.DEFAULT_GOAL := all
.PHONY: all
