ifeq (${BSRC_DEBUG},true)
  TARGET := DEBUG
  FLAGS += -DDBG_PRINT
  CFLAGS += -D__SRC_LEVEL_DEBUG
else
  TARGET := RELEASE
endif

PROJ := SigmaBootPkg
DSC := SigmaBootPkg/Boot.dsc
PLATFORM_NAME := $(shell awk -F '=' '/^ *PLATFORM_NAME *=/ {gsub(/ /, "", $$2); printf $$2}' ${DSC})

prepare:
	@ln -snf $(abspath SigmaBootPkg) Edk2/SigmaBootPkg

build: prepare
	@mkdir -p Edk2/Conf
	export WORKSPACE=$(abspath Edk2) && \
	export GCC5_BIN=$(CROSS_COMPILE) && \
	source Edk2/BaseTools/BuildEnv && \
		if ! build --help > /dev/null; then \
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
		-DOUTPUT=$(BUILD) \
		-DCFLAGS="$(CFLAGS)" 2>&1
	@$(UTILS)/chkmodify_unlock.sh $(PROJ)

update: BUILD:=$(BUILD)/SIGMA_$(EFI_ARCH)
update: EFIEXE:=$(BUILD)/$(TARGET)_$(TOOLCHAIN)/$(EFI_ARCH)/$(PLATFORM_NAME)
update:
	@if ( ! test -f $(EFIEXE).efi ) || \
		( ! $(UTILS)/chkmodify.sh $(PROJ) ); then \
		make build BUILD=$(BUILD) \
	;fi

clean:
	rm -f Edk2/SigmaBootPkg

.PHONY: prepare build update clean

# installation
R = $(BUILD)/SIGMA_$(EFI_ARCH)/$(TARGET)_$(TOOLCHAIN)/$(EFI_ARCH)/$(PLATFORM_NAME).efi:/EFI/BOOT/$(EFI_RMF_NAME)
