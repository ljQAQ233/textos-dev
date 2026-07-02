APP_MODS := test strace
APP_LIBS := libc libm lvgl

INCLUDE =
LIBRARY =

# Build INCLUDE and LIBRARY from each library's inc.mk.
# Every inc.mk may append to INCLUDE (header paths)
# and LIBRARY (library paths), used by CFLAGS/LDFLAGS below
include $(patsubst %,$(SRC_DIR)/app/%/inc.mk,$(APP_LIBS))

CFLAGS = -g \
  $(addprefix -I,${INCLUDE})

LDFLAGS = \
  -z noexecstack \
  -L $(ROOT_LIB)

# NOTE: exporting expands CFLAGS to a static string; sub-makes
# that append to INCLUDE later will *not* retroactively affect it.
export INCLUDE LIBRARY CFLAGS LDFLAGS
