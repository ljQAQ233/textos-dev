BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})
MAINHDR := $(BUILD)/include/openlibm/openlibm.h
OLM_MAKE := cd openlibm && ARCH=$(ARCH) $(MAKE) CC=$(APP_OUTPUT)/libc/cc 

# compile again only when .stamp is updated
$(MAINHDR): .stamp
	@mkdir -p $(BUILD)
	@$(OLM_MAKE) clean
	@$(OLM_MAKE) all
	@$(OLM_MAKE) prefix=$(BUILD) install
	
	cp -L $(BUILD)/lib/libopenlibm.a $(BUILD)/libm.a
	cp -L $(BUILD)/lib/libopenlibm.so $(BUILD)/libm.so
	patchelf $(BUILD)/libm.so --set-soname libm.so
	cp -L $(BUILD)/libm.so $(ROOT_LIB)/libm.so
	@touch $@

.stamp:
	touch $@
