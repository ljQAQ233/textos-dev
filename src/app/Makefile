SHELL  := bash

include ../config/app.mk

ELFS := $(patsubst %.c,%,$(wildcard *.c))
ELFS := $(addprefix ${APP_OUTPUT}/,${ELFS})

MODS := test
LIBS := libc libm lvgl

app: libs mods $(ELFS)
	@mkdir -p $(ROOT)/bin
	@cp $(ELFS) $(ROOT)/bin

libs:
	@for i in ${LIBS}; do cd $$i; make || exit; cd ..; done

mods:
	@for i in ${MODS}; do cd $$i; make || exit; cd ..; done

clean:
	rm -rf $(APP_OUTPUT)

clean-elfs:
	rm -f $(ELFS)

$(APP_OUTPUT)/%: %.c $(LIBS:=/.stamp)
	@mkdir -p $(@D)
	@$(APP_OUTPUT)/libc/cc -static $(CFLAGS) $< $(addprefix $(APP_OUTPUT)/,$(shell ${UTILS}/fix_libs.sh $<)) -o $@ -L$(APP_OUTPUT)/libc -lc -ldl
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: app libs mods clean clean-elfs

# special targets:
# - use `make ls` to generate rather than `make path/to/build/ls`!
%: %.c
	@$(MAKE) $(APP_OUTPUT)/$@
