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

ovmf: export BOOT_OUTPUT:=$(BOOT_OUTPUT)/OVMF_$(ARCH)
ovmf:
	@mkdir -p Edk2/Conf
	@echo -e "Update configures for compliler...$(TARGET)\n"
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
	@cp -f `find $(BOOT_OUTPUT) -name "OVMF.fd" | grep "$(TARGET)_$(TOOLCHAIN)" | grep $(ARCH) | awk 'NR==1{print $$0}'` $(BASE)/OVMF_$(TARGET)_$(ARCH).fd
	@echo

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

update:
	@if find $(BOOT_OUTPUT) -iname $(PLATFORM_NAME).efi \
	  | grep "$(TARGET)_$(TOOLCHAIN)" | grep "$(ARCH)"  \
	  | awk 'NR==1{print $$0}' | xargs test -z; then    \
		make -C .. boot \
	;elif ! $(UTILS)/chkmodify.sh $(PROJ) >/dev/null 2>&1;then \
		make -C .. boot \
	;fi
	@$(SUDO) cp -rf "`find $(BOOT_OUTPUT) -iname $(PLATFORM_NAME).efi | grep "$(TARGET)_$(TOOLCHAIN)" | grep "$(ARCH)" | awk 'NR==1{print $$0}'`" $(BOOT_EXEC)

clean:
	rm -rf Edk2/SigmaBootPkg

.PHONY: prepare ovmf build update clean
