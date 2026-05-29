INCLUDE := \
  $(SRC_DIR)/include \
  $(SRC_DIR)/include/arch/$(ARCH) \
  $(SRC_DIR)/app/lvgl \
  $(APP_OUTPUT)/libm/include/openlibm

LIBRARY := \
  $(APP_OUTPUT)/libc \
  $(APP_OUTPUT)/libm

CFLAGS := -g \
  $(addprefix -I,${INCLUDE})

LDFLAGS := \
  -z noexecstack \
  -L$(ROOT)/lib

export INCLUDE LIBRARY CFLAGS LDFLAGS
