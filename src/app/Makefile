BUILD := $(BUILD)/app

export ORIG_CC := $(CC)
CC := $(BUILD)/libc/cc

APP_MODS := test strace lua
APP_LIBS := libc libm lvgl

INCLUDE =
LIBRARY =

# Build INCLUDE and LIBRARY from each library's inc.mk.
# Every inc.mk may append to INCLUDE (header paths)
# and LIBRARY (library paths), used by CFLAGS/LDFLAGS below
-include $(patsubst %,%/inc.mk,$(APP_LIBS))

CFLAGS = -g \
  $(addprefix -I,${INCLUDE})

LDFLAGS = \
  -z noexecstack \
  $(addprefix -L,$(LIBRARY))

# NOTE: exporting expands CFLAGS to a static string; sub-makes
# that append to INCLUDE later will *not* retroactively affect it.
export INCLUDE LIBRARY CFLAGS LDFLAGS

ELFS := $(patsubst %.c,%,$(wildcard *.c))
ELFS := $(addprefix ${BUILD}/,${ELFS})

MODS = $(APP_LIBS) $(APP_MODS)

app: all $(ELFS)

clean:
	rm -rf $(BUILD)

clean-elfs:
	rm -f $(ELFS)

include cmd.mk

$(BUILD)/%: %.c $(APP_LIBS:=/.stamp)
	@$(call compile-cc,$<,$@,-static)

$(BUILD)/dwm: LDLIBS += -llvgl

.PHONY: app clean clean-elfs

# special targets:
# - use `make ls` to generate rather than `make path/to/build/ls`!
%: %.c
	@$(MAKE) $(BUILD)/$@

$(MODS):
	$(MAKE) -C $@

include ../make/subdir.mk
include ../make/all-node.mk
include ../make/install-node.mk
include ../make/install-leaf.mk

.PHONY: all
