ARCH   := X64

export ARCH

include config/main.mk
include config/image.mk

all: _boot image

include config/qemu.mk

boot:
	@make -C boot build

ovmf:
	@make -C boot TARGET=RELEASE ovmf
	@echo "OVMF is up-to-date\n"


image:
	@mkdir -p $(IMG_MDIR)
	@if ! $(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		echo -e "Created an image : $(IMG_OUTPUT)" \
		;dd if=/dev/zero of=$(IMG_OUTPUT) bs=1M count=$(IMG_SIZE) > /dev/null 2>&1 \
		;mkfs.fat -F 16 $(IMG_OUTPUT) > /dev/null 2>&1 \
		;$(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) \
	;fi
	
	@echo "Copying files..."
	@$(SUDO) mkdir -p $(IMG_MDIR)/EFI/Boot/
	@$(SUDO) cp $(BOOT_EXEC) $(IMG_MDIR)/EFI/Boot/$(OUT_EFI)
	
	@echo "Image was done!"
	@while $(SUDO) umount $(IMG_MDIR) 2>/dev/null; \
		do sleep 1	\
	;done
	@$(SUDO) rm -rf $(IMG_MDIR)
	@$(SUDO) chmod a+rw $(IMG_OUTPUT)

compile_commands.json: clean
	@bear --output $(UTILS)/compile_commands.json -- make clean all
	@$(UTILS)/fix_clcmd.sh $(UTILS)/compile_commands.json
	@echo "compile_commands.json is up-to-date"

_boot:
	@make -C boot update

qemu: all
	$(QEMU) $(QEMU_FLAGS_RUN)

diskmu:
	@mkdir -p $(IMG_MDIR)
	@if ! $(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		$(SUDO) umount $(IMG_MDIR) \
		;rm -rf $(IMG_MDIR) \
		;echo "Umounted the disk and removed the dir of the disk." \
	;else \
		echo "Created an dir for the disk and mounted the disk." \
	;fi

clean:
	make -C boot clean
	find . -iname "*.status_log" | xargs rm -f
	find . -iname "*.status_lock" | xargs rm -f
	rm -rf $(OUTPUT)


.PHONY: all _boot boot image \
		qemu \
		diskmu clean \
		compile_commands.json 

