ARCH           ?= X64
TOOLCHAIN      ?= GCC5

# For EDK2 build command

INCLUDE   := $(SRC_DIR)/Include
FLAGS     := $(_SKIP_AUTOGEN)
# For EDK2 Build Toolchains
CFLAGS    := $(addprefix -I,${INCLUDE})

ifeq (${BOOT_DEBUG},true)
  CFLAGS  += -D__SRC_LEVEL_DEBUG
endif
# For Gcc

ifeq (${BOOT_DEBUG},true)
  FLAGS += -DBOOT_DEBUG
 # For Source Level Debug and others,also a Macro in .dsc file
endif

PROJ := SigmaBootPkg
DSC  := $(PROJ)/Boot.dsc
# For EDK2 build command,means what to build
_PLATFORM_NAME := $(shell bash ${UTILS}/BootGetPlatformName.sh ${DSC})
# Get PlatformName
VERSION := $(shell LANG= date +"%Y%m%d")
# For Project Description File (.dsc), ThisFmt : Date as number

export VERSION DSC TOOLCHAIN FLAGS CFLAGS OUTPUT TARGET SRC_DEBUG _PLATFORM_NAME
# export Options
