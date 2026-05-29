LVGL_SHORT_PATH ?= lvgl
LVGL_CONF ?= $(shell pwd)/lv_conf.h

CC := $(ORIG_CC)
TARG := lvgl.o
SRCS := $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.S')
SRCS += $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.c')
SRCS += $(shell find $(LVGL_SHORT_PATH)/src -type f -name '*.cpp')

LVGL_PATH := $(realpath ${LVGL_SHORT_PATH})

AFLAGS   := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""
CFLAGS   := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""
CXXFLAGS := -static -fno-stack-check -fno-stack-protector -I$(LVGL_PATH) -DLV_CONF_PATH="\"$(LVGL_CONF)\""

include ../all.mk

$(APP_OUTPUT)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo -e "\033[032m   CXX \033[0m $<"

$(APP_OUTPUT)/%.S.o: %.S
	@mkdir -p $(@D)
	@$(CC) $(AFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"
