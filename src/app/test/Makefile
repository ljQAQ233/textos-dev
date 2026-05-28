TEST_SRCS := \
  $(wildcard sys_*.c) \
  $(wildcard libc_*.c)

SRCS := $(TEST_SRCS)
SRCS += runtest.c
SRCS += unity/unity.c

TARG := runtest

GENS := $(wildcard *.py)
BINS := $(wildcard bin/*.c)
GTARS := \
  $(GENS:%.py=auto/%.h) \
  auto/unity.h
BTARS := $(BINS:%.c=)

CC := $(APP_OUTPUT)/libc/cc
CFLAGS += -Iunity
CFLAGS += -DUNITY_INCLUDE_CONFIG_H
CFLAGS += -lm -static

include ../all.mk

all: $(GTARS) $(TARG)
	@mkdir -p $(ROOT)/test
	@cp $^ $(ROOT)/test

auto/%.h: %.py
	@mkdir -p $(@D)
	@python $< > $@
	@echo -e "\033[032m   PY  \033[0m $<"

# #define TESTS \
# _(function, name, line)
auto/unity.h: $(TEST_SRCS)
	@mkdir -p $(@D)
	@mkdir -p $(@D)
	@printf '#define TESTS' > $@
	@grep '//!register=' $^ -Hn | \
		sed 's/\/\/!register=//' | \
		awk -F: '{printf " \\\n_(\"%s\",%s,\"%s\",%d)",$$1,$$3,$$3,$$2}' >> $@
	@printf '\n' >> $@
	@echo -e "\033[032m   OK  \033[0m $@"

.DEFAULT_GOAL := all
.PHONY: all
