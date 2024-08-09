OUTPUT    ?= .
SHELL     := bash

include ../config/boot.mk

ifeq (${BSRC_DEBUG},true)
 TARGET := DEBUG
else
 TARGET := RELEASE
endif

prepare:
	@ln -snf $(abspath SigmaBootPkg) Edk2/SigmaBootPkg

ovmf: BOOT_OUTPUT:=$(BOOT_OUTPUT)/OVMF_$(ARCH)
ovmf:
	@mkdir -p Edk2/Conf
	@echo -e "Update configures for compliler...\n"
	@export WORKSPACE=$(abspath Edk2) && \
		source Edk2/BaseTools/BuildEnv && \
		if ! build --help > /dev/null;then \
			rm -rf Conf/tools_def.txt \
			Conf/target.txt \
			Conf/build_rule.txt \
		;fi
	
	@echo -e "Start to build Boot Module...\n"
	@export WORKSPACE=$(abspath Edk2) && \
		source Edk2/BaseTools/BuildEnv > /dev/null && \
		build $(FLAGS) -p OvmfPkg/OvmfPkg.dsc \
		-a $(ARCH) \
		-t $(TOOLCHAIN) \
		-b $(TARGET) \
		-DOUTPUT=$(BOOT_OUTPUT) \
		-DARCH=$(ARCH)
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF.fd $(BASE)/OVMF_$(TARGET)_$(ARCH).fd
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF_CODE.fd $(BASE)/OVMF_$(TARGET)_$(ARCH).code
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF_VARS.fd $(BASE)/OVMF_$(TARGET)_$(ARCH).vars
	@echo

build: BOOT_OUTPUT:=$(BOOT_OUTPUT)/SIGMA_$(ARCH)
build: prepare
	@mkdir -p Edk2/Conf
	@echo -e "Update configures for compliler...\n"
	@export WORKSPACE=$(abspath Edk2) && \
	source Edk2/BaseTools/BuildEnv && \
		if ! build --help > /dev/null;then \
			rm -rf Conf/tools_def.txt \
			Conf/target.txt \
			Conf/build_rule.txt \
		;fi
	
	@echo -e "Start to build Boot Module...\n"
	@export WORKSPACE=$(abspath Edk2) && \
	source Edk2/BaseTools/BuildEnv > /dev/null && \
		build $(FLAGS) -p $(DSC) \
		-a $(ARCH) \
		-t $(TOOLCHAIN) \
		-b $(TARGET) \
		-DOUTPUT=$(BOOT_OUTPUT) \
		-DCFLAGS="$(CFLAGS)" 2>&1
	@$(UTILS)/chkmodify_unlock.sh $(PROJ)
	@echo

update: BOOT_OUTPUT:=$(BOOT_OUTPUT)/SIGMA_$(ARCH)
update: EXEC_OUTPUT:=$(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/$(ARCH)/$(PLATFORM_NAME)
update:
	@if ! test -f $(EXEC_OUTPUT); then \
		make -C .. boot \
	;elif ! $(UTILS)/chkmodify.sh $(PROJ) >/dev/null 2>&1;then \
		make -C .. boot \
	;fi
	cp -rf $(EXEC_OUTPUT).efi $(BOOT_EXEC)

clean:
	rm -rf Edk2/SigmaBootPkg

.PHONY: prepare ovmf build update clean
