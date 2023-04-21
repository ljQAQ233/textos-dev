ARCH   := X64

export ARCH

include Config/Main.mk
include Config/Image.mk

.PHONY: all
all: _BootUpdate Image

include Config/Qemu.mk

.PHONY: Boot
Boot:
	@cp -ra SigmaBootPkg Boot/
	@echo -e "Start to Compile Boot Module...\n"
	@make -C Boot
	@echo -e "Boot Module was completed\n"

.PHONY: Image
Image:
	@mkdir -p $(IMG_MDIR)
	@if ! $(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		echo -e "Create an image : $(IMG_OUTPUT)" \
		;dd if=/dev/zero of=$(IMG_OUTPUT) bs=1M count=$(IMG_SIZE) > /dev/null 2>&1 \
		;mkfs.fat -F 16 $(IMG_OUTPUT) > /dev/null 2>&1 \
		;$(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) \
	;fi
	
	@echo "Copying files for booting..."
	@$(SUDO) mkdir -p $(IMG_MDIR)/EFI/Boot/
	@$(SUDO) cp $(BOOT_EXEC) $(IMG_MDIR)/EFI/Boot/$(OUT_EFI)
	
	@echo "Image was done!"
	@while $(SUDO) umount $(IMG_MDIR) 2>/dev/null; \
		do sleep 1	\
	;done
	@$(SUDO) rm -rf $(IMG_MDIR)
	@$(SUDO) chmod a+rw $(IMG_OUTPUT)

.PHONY: compile_commands.json
compile_commands.json: clean
	@echo -e "Generating compile_commands.json ..."
	@bear -o compile_commands_raw.json make clean all
	@grep -v "\-maccumulate-outgoing-args" compile_commands_raw.json  > $(UTILS)/compile_commands.json
	@rm compile_commands_raw.json
	@echo "compile_commands.json was up-to-date"

.PHONY: _BootUpdate
_BootUpdate:
	@cp -ra SigmaBootPkg Boot/
	@make -C Boot Update

.PHONY: qemu
qemu: all
	$(QEMU) $(QEMU_FLAGS_RUN)

.PHONY: diskmu
diskmu:
	@mkdir -p $(IMG_MDIR)
	@if ! $(SUDO) mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		$(SUDO) umount $(IMG_MDIR) \
		;rm -rf $(IMG_MDIR) \
		;echo "Umounted the disk and removed the dir of the disk." \
	;else \
		echo "Created an dir for the disk and mounted the disk." \
	;fi

.PHONY: clean
clean:
	find . -iname "*.status_log" | xargs rm -f
	rm -rf Boot/Conf
	rm -rf Boot/SigmaBootPkg
	rm -rf $(OUTPUT)
