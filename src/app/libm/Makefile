BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})
MAINHDR := $(BUILD)/include/openlibm/openlibm.h

# compile again only when .stamp is updated
$(MAINHDR): .stamp
	@mkdir -p $(BUILD)
	@cd openlibm && ARCH= $(MAKE) clean
	@cd openlibm && ARCH= $(MAKE) CC=$(APP_OUTPUT)/libc/cc
	@cd openlibm && ARCH= $(MAKE) prefix=$(BUILD) install
	
	@mkdir -p $(ROOT)/lib
	@cp -L $(BUILD)/lib/libopenlibm.a $(BUILD)/libm.a
	@cp -L $(BUILD)/lib/libopenlibm.so $(BUILD)/libm.so
	@cp -L $(BUILD)/lib/libopenlibm.so $(ROOT)/lib/libm.so
	@touch $@

.stamp:
	touch .stamp
