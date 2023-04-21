ARCH           ?= X64
TOOLCHAIN      ?= GCC5

INCLUDE   := $(SRC_DIR)/include \
			 $(SRC_DIR)/include/boot
FLAGS     := $(_SKIP_AUTGEN)
# For EDK2 Build Toolchains
CFLAGS    := $(addprefix -I,${INCLUDE})

PROJ := SigmaBootPkg
DSC  := SigmaBootPkg/Boot.dsc
PLATFORM_NAME := $(shell bash ${UTILS}/dsc_platform.sh ${DSC})

# For Project Description File (.dsc), ThisFmt : Date as number
VERSION := $(shell LANG= date +"%Y%m%d")

export TOOLCHAIN FLAGS CFLAGS
export PROJ DSC PLATFORM_NAME VERSION
