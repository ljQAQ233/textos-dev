ifeq (${BSRC_DEBUG},true)
  TARGET := DEBUG
  FLAGS += -DDBG_PRINT
  CFLAGS += -D__SRC_LEVEL_DEBUG
else
  TARGET := RELEASE
endif

PLATFORM_NAME := $(shell awk -F '=' '/^ *PLATFORM_NAME *=/ {gsub(/ /, "", $$2); printf $$2}' ${DSC})

prepare:
	@ln -snf $(abspath SigmaBootPkg) Edk2/SigmaBootPkg

build: prepare
	@mkdir -p Edk2/Conf
	@echo -e "Update configures for compliler...\n"
	export WORKSPACE=$(abspath Edk2) && \
	export GCC5_BIN=$(CROSS_COMPILE) && \
	source Edk2/BaseTools/BuildEnv && \
		if ! build --help > /dev/null;then \
			rm -rf Conf/tools_def.txt \
			Conf/target.txt \
			Conf/build_rule.txt \
		;fi

	@echo -e "Start to build Boot Module...\n"
	export WORKSPACE=$(abspath Edk2) && \
	export GCC5_BIN=$(CROSS_COMPILE) && \
	source Edk2/BaseTools/BuildEnv > /dev/null && \
		build $(FLAGS) -p $(DSC) \
		-a $(EFI_ARCH) \
		-t $(TOOLCHAIN) \
		-b $(TARGET) \
		-DOUTPUT=$(BOOT_OUTPUT) \
		-DCFLAGS="$(CFLAGS)" 2>&1
	@$(UTILS)/chkmodify_unlock.sh $(PROJ)

update: BOOT_OUTPUT:=$(BOOT_OUTPUT)/SIGMA_$(EFI_ARCH)
update: EXEC_OUTPUT:=$(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/$(EFI_ARCH)/$(PLATFORM_NAME)
update:
	@if ( ! test -f $(EXEC_OUTPUT).efi ) || \
		( ! $(UTILS)/chkmodify.sh $(PROJ) ); then \
		make build \
	;fi
	@cp -f $(EXEC_OUTPUT).efi $(BOOT_EXEC)
	@mkdir -p $(ROOT)/EFI/Boot
	@cp -f $(BOOT_EXEC) $(ROOT)/EFI/Boot

clean:
	rm -f Edk2/SigmaBootPkg

.PHONY: prepare build update clean
