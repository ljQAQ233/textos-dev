BUILD := $(BUILD)/libm
OLM_MAKE := cd openlibm && ARCH=$(ARCH) $(MAKE)

all: $(BUILD)/libm.a

$(BUILD)/libm.a: $(BUILD)/lib/libopenlibm.a
	patchelf $(BUILD)/libm.so --set-soname libm.so
	@touch .stamp

$(BUILD)/lib/libopenlibm.a: openlibm/libopenlibm.a
	@mkdir -p $(BUILD)
	@$(OLM_MAKE) prefix=$(BUILD) install
	cp -L $(BUILD)/lib/libopenlibm.a $(BUILD)/libm.a
	cp -L $(BUILD)/lib/libopenlibm.so $(BUILD)/libm.so

openlibm/libopenlibm.a:
	@$(OLM_MAKE) all

.PHONY: all

include ../../make/install-leaf.mk
