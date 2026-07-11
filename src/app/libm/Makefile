BUILD := $(BUILD)/libm
OLM_MAKE := cd openlibm && ARCH=$(ARCH) $(MAKE) CC=$(BUILD)/libc/cc 

all:
	@mkdir -p $(BUILD)
	@$(OLM_MAKE) all
	@$(OLM_MAKE) prefix=$(BUILD) install
	
	cp -L $(BUILD)/lib/libopenlibm.a $(BUILD)/libm.a
	cp -L $(BUILD)/lib/libopenlibm.so $(BUILD)/libm.so
	patchelf $(BUILD)/libm.so --set-soname libm.so

.PHONY: all

include ../../make/install-leaf.mk
