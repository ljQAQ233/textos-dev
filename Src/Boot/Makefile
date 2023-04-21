OUTPUT    ?= .
SHELL     := bash

-include ../Config/Boot.mk

TARGET := RELEASE

.PHONY: BootLoader
BootLoader: Build Update

.PHONY: Build
Build:
	@mkdir -p Conf
	@echo -e "Update configures for compliler...\n"
	@source ./BaseTools/BuildEnv && \
		if ! build --help > /dev/null;then \
			rm -rf Conf/tools_def.txt \
			Conf/target.txt \
			Conf/build_rule.txt \
		;fi
	
	@echo -e "Start to build Boot Module...\n"
	@source ./BaseTools/BuildEnv > /dev/null && \
		build $(FLAGS) -p $(DSC) \
		-a $(ARCH) \
		-t $(TOOLCHAIN) \
		-b $(TARGET) \
		-DOUTPUT=$(BOOT_OUTPUT) \
		-DCFLAGS="$(CFLAGS)" 2>&1
	@echo

.PHONY: Update
Update:
	@if find $(BOOT_OUTPUT) -iname $(_PLATFORM_NAME).efi 2>/dev/null | grep "$(TARGET)_$(TOOLCHAIN)" | grep "$(ARCH)" | awk 'NR==1{print $$0}' | xargs test -z;then \
		make -C .. Boot \
	;fi
	@$(SUDO) cp -rf "`find $(BOOT_OUTPUT) -iname $(_PLATFORM_NAME).efi | grep "$(TARGET)_$(TOOLCHAIN)" | grep "$(ARCH)" | awk 'NR==1{print $$0}'`" $(BOOT_EXEC)
