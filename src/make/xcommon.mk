# 转化为 edk2 配置
efi_arch_x86 = IA32
efi_arch_x86_64 = X64
EFI_ARCH := $(efi_arch_$(ARCH))

efi_rmf_name_X64  = BOOTX64.EFI
efi_rmf_name_IA32 = BOOTIA32.EFI
EFI_RMF_NAME = $(efi_rmf_name_$(EFI_ARCH))

export EFI_ARCH EFI_RMF_NAME

ifneq ($(shell git rev-parse --is-inside-work-tree 2>/dev/null),true)
  GIT_HASH := unknown
else
  GIT_HASH := $(shell git rev-parse --short HEAD)
endif

export GIT_HASH
