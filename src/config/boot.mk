INCLUDE := $(SRC_DIR)/include \
		   $(SRC_DIR)/include/boot
FLAGS :=
CFLAGS := $(addprefix -I,${INCLUDE})

# boot version
VERSION := $(shell LANG= date +"%Y%m%d")

export FLAGS CFLAGS VERSION

# === edk2-specified ===
PROJ := SigmaBootPkg
DSC  := SigmaBootPkg/Boot.dsc

export PROJ DSC PLATFORM_NAME

# === emulti-specified ===
