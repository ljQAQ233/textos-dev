BUILD := $(BUILD)/lvgl

LVGL_SHORT_PATH ?= lvgl
LVGL_CONF ?= $(shell pwd)/lv_conf.h

CC := $(ORIG_CC)
SRCS := $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.S')
SRCS += $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.c')
SRCS += $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.cpp')

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${BUILD}/,${OBJS})

LVGL_PATH := $(realpath ${LVGL_SHORT_PATH})

AFLAGS   := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""
CFLAGS   := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""
CXXFLAGS := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""

include ../cmd.mk

all: $(BUILD)/liblvgl.a

$(BUILD)/liblvgl.a: $(BUILD)/lvgl.o | $(BUILD)
	$(call compile-ar,$<,$@)

$(BUILD):
	@mkdir -p $@

$(BUILD)/lvgl.o: $(OBJS)
	$(call compile-merge,$^,$@)

$(BUILD)/%.c.o: %.c
	$(call compile-obj,$<,$@)

$(BUILD)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo -e "\033[032m   CXX \033[0m $<"

$(BUILD)/%.S.o: %.S
	@mkdir -p $(@D)
	@$(CC) $(AFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

.PHONY: all

install:
	@
