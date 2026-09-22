INST_CUSTOM = $(BOOTDIR)/kernel.elf

$(BOOTDIR)/kernel.elf: \
	$(BUILD)/kernel.elf
	$(INSTALL) -D $< $@
