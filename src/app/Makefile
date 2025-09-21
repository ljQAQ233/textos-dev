SHELL  := bash

include ../config/app.mk

elfs := $(wildcard *.c)
elfs := $(elfs:.c=.elf)
elfs := $(addprefix ${APP_OUTPUT}/,${elfs})

MODS := \
	libc \
	lvgl

app: mods $(elfs)

mods:
	@for i in ${MODS}; do cd $$i; make || exit; cd ..; done

clean:
	rm -rf $(APP_OUTPUT)

# Make rules and printing formats (top-level)

$(APP_OUTPUT)/%.elf: %.c $(MODS:=/.stamp)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -static $(LIBC) $(addprefix $(APP_OUTPUT)/,$(shell ${UTILS}/fix_libs.sh $<)) -o $@ -L$(APP_OUTPUT)/libc -lc -ldl
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: app mods clean
