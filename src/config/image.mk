IMG = $(OUTPUT)/image.img

export IMG

LOOP=$(shell sudo losetup -j ${IMG} | awk -F ':' '{print $$1}')
ifeq (${LOOP},)
  	LOOP=$(shell sudo losetup -f)

diskmu:
	@echo "mounting disk..."
	mkdir -p $(MNT1)
	mkdir -p $(MNT2)
	sudo losetup -P $(LOOP) $(IMG)
	sudo mount $(LOOP)p1 $(MNT1)
	sudo mount $(LOOP)p2 $(MNT2)
else

diskmu:
	@echo "unmounting disk..."
	sudo umount $(MNT1)
	sudo umount $(MNT2)
	sudo losetup -d $(LOOP)
	sudo rm -rf $(OUTPUT)/image

endif

MNT1:=$(OUTPUT)/image/$(notdir ${LOOP})p1
MNT2:=$(OUTPUT)/image/$(notdir ${LOOP})p2

$(IMG):
	mkdir -p $(MNT1)
	mkdir -p $(MNT2)
	
	dd if=/dev/zero of=$(IMG) bs=1M count=128
	sfdisk $(IMG) < utils/disk.conf
	
	sudo losetup -P $(LOOP) $(IMG)
	sudo mkfs.fat -F 32 $(LOOP)p1
	sudo mkfs.fat -F 32 $(LOOP)p2
	
	sudo mount $(LOOP)p1 $(MNT1)
	sudo mount $(LOOP)p2 $(MNT2)
	
	sudo mkdir -p $(MNT1)/EFI/Boot
	sudo cp -r $(BOOT_EXEC) $(MNT1)/EFI/Boot/$(OUT_EFI)
	sudo cp -r resource/* $(MNT1)
	sudo cp -r $(KERNEL_EXEC) $(MNT1)
	sudo cp -r $(APP_OUTPUT)/*.elf $(MNT1)
	
	sudo umount $(MNT1)
	sudo umount $(MNT2)
	
	sudo losetup -d $(LOOP)
	
	sudo chmod a+rw $(MNT1)
	sudo chmod a+rw $(MNT2)
	sudo rm -rf $(OUTPUT)/image