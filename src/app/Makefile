SHELL := bash

export ORIG_CC := $(CC)
CC := $(APP_OUTPUT)/libc/cc

include ../config/app.mk

ELFS := $(patsubst %.c,%,$(wildcard *.c))
ELFS := $(addprefix ${APP_OUTPUT}/,${ELFS})

app: libs mods $(ELFS)
	@cp $(ELFS) $(ROOT_BIN)

libs:
	@for i in ${APP_LIBS}; do cd $$i; make || exit; cd ..; done

mods:
	@for i in ${APP_MODS}; do cd $$i; make || exit; cd ..; done

clean:
	rm -rf $(APP_OUTPUT)

clean-elfs:
	rm -f $(ELFS)

$(APP_OUTPUT)/%: %.c $(APP_LIBS:=/.stamp)
	@mkdir -p $(@D)
	@$(CC) -static $(CFLAGS) $< $(addprefix $(APP_OUTPUT)/,$(shell ${UTILS}/fix_libs.sh $<)) -o $@ -L$(APP_OUTPUT)/libc -lc -ldl
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: app libs mods clean clean-elfs

# special targets:
# - use `make ls` to generate rather than `make path/to/build/ls`!
%: %.c
	@$(MAKE) $(APP_OUTPUT)/$@

# 子模块路由：include ../config/subdir.mk 提供 @ 语法
$(APP_LIBS) $(APP_MODS):
	$(MAKE) -C $@

include ../config/subdir.mk
