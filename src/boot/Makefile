SHELL := bash

# GCC5 的配置 对于高版本编译器 依然可以 "兼容";
# NOTE: 不要修改这段代码!!!
TOOLCHAIN := GCC5

include ../config/boot.mk
# 每一个 BOOT_MODE 都对应一个 .mk 文件
# 每一个 .mk 都必须实现 build, update, clean, prepare 目标
# 分别对应 正式构建, 检测有无改动并触发构建, 清理, 构建前准备
# 由于 OVMF 与 BOOT_MODE 没有太多关联, 因为你都希望构建了,
# 那么我阻止也没用呀, 所以安排到了这里
include ./$(BOOT_MODE).mk

ovmf: BOOT_OUTPUT:=$(BOOT_OUTPUT)/OVMF_$(EFI_ARCH)
ovmf:
	mkdir -p Edk2/Conf
	@echo -e "Update configures for compliler...\n"
	export WORKSPACE=$(abspath Edk2) && \
		source Edk2/BaseTools/BuildEnv && \
		if ! build --help > /dev/null;then \
			rm -rf Conf/tools_def.txt \
			Conf/target.txt \
			Conf/build_rule.txt \
		;fi

	@echo -e "Start to build Boot Module...\n"
	export WORKSPACE=$(abspath Edk2) && \
		source Edk2/BaseTools/BuildEnv > /dev/null && \
		build $(FLAGS) -p OvmfPkg/OvmfPkg.dsc \
		-a $(EFI_ARCH) \
		-t $(TOOLCHAIN) \
		-b $(TARGET) \
		-DOUTPUT=$(BOOT_OUTPUT) \
		-DARCH=$(EFI_ARCH)
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF.fd $(BASE)/OVMF_$(TARGET)_$(EFI_ARCH).fd
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF_CODE.fd $(BASE)/OVMF_$(TARGET)_$(EFI_ARCH).code
	cp -f $(BOOT_OUTPUT)/$(TARGET)_$(TOOLCHAIN)/FV/OVMF_VARS.fd $(BASE)/OVMF_$(TARGET)_$(EFI_ARCH).vars

.PHONY: ovmf
.DEFAULT_GOAL := update
