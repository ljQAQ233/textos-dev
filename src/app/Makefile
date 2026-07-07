SHELL := bash

export ORIG_CC := $(CC)
CC := $(APP_OUTPUT)/libc/cc

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
  -L $(ROOT_LIB)

# NOTE: exporting expands CFLAGS to a static string; sub-makes
# that append to INCLUDE later will *not* retroactively affect it.
export INCLUDE LIBRARY CFLAGS LDFLAGS

ELFS := $(patsubst %.c,%,$(wildcard *.c))
ELFS := $(addprefix ${APP_OUTPUT}/,${ELFS})

MODS = $(APP_LIBS) $(APP_MODS)

app: all $(ELFS)
	@cp $(ELFS) $(ROOT_BIN)

clean:
	rm -rf $(APP_OUTPUT)

clean-elfs:
	rm -f $(ELFS)

$(APP_OUTPUT)/%: %.c $(APP_LIBS:=/.stamp)
	@mkdir -p $(@D)
	@$(CC) -static $(CFLAGS) $< $(addprefix $(APP_OUTPUT)/,$(shell ${UTILS}/fix_libs.sh $<)) -o $@ -L$(APP_OUTPUT)/libc -lc -ldl
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: app clean clean-elfs

# special targets:
# - use `make ls` to generate rather than `make path/to/build/ls`!
%: %.c
	@$(MAKE) $(APP_OUTPUT)/$@

# 子模块路由：include ../subdir/subdir.mk 提供 @ 语法
$(APP_LIBS) $(APP_MODS):
	$(MAKE) -C $@

include ../make/subdir.mk
include ../make/all-node.mk
include ../make/install-node.mk

.PHONY: all
