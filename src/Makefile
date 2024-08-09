ARCH   := X64

export ARCH

include config/main.mk
include config/image.mk

include config/udk.mk

all: _boot kernel app image

include config/qemu.mk

boot:
	@make -C boot build

ovmf:
	@make -C boot TARGET=RELEASE ovmf
	@make -C boot TARGET=DEBUG FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE" ovmf
	@make -C boot TARGET=NOOPT FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE -DSOURCE_DEBUG_ENABLE=TRUE" ovmf
	@echo "OVMF is up-to-date\n"

kernel:
	@make -C kernel kernel

app:
	@make -C app app

image:
	@mkdir -p $(IMG_MDIR)
	@if ! sudo mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		echo -e "Created an image : $(IMG_OUTPUT)" \
		;dd if=/dev/zero of=$(IMG_OUTPUT) bs=1M count=$(IMG_SIZE) > /dev/null 2>&1 \
		;mkfs.fat -F 32 --mbr=y $ $(IMG_OUTPUT) > /dev/null 2>&1 \
		;sudo mount $(IMG_OUTPUT) $(IMG_MDIR) \
	;fi
	
	@echo "Copying files..."
	@sudo mkdir -p $(IMG_MDIR)/EFI/Boot/
	@sudo cp $(BOOT_EXEC) $(IMG_MDIR)/EFI/Boot/$(OUT_EFI)
	@sudo cp -rf resource/* $(IMG_MDIR)/
	@sudo cp $(KERNEL_EXEC) $(IMG_MDIR)/
	@sudo cp $(APP_OUTPUT)/* $(IMG_MDIR)/
	
	@echo "Image was done!"
	@while sudo umount $(IMG_MDIR) 2>/dev/null; \
		do sleep 1	\
	;done
	@sudo rm -rf $(IMG_MDIR)
	@sudo chmod a+rw $(IMG_OUTPUT)

compile_commands.json: clean
	@bear --output $(UTILS)/compile_commands.json -- make clean all
	@$(UTILS)/fix_clcmd.sh $(UTILS)/compile_commands.json
	@echo "compile_commands.json is up-to-date"

_boot:
	@make -C boot update

qemu: all
	$(QEMU) $(QEMU_FLAGS_RUN)

qemug: all
	$(QEMU) $(QEMU_FLAGS_KDBG)

qemubg: export BSRC_DEBUG:=true
qemubg: export PIPE:=$(shell cat $(UDK_DBG_CFG) | grep -E "^[P ]ort" | awk '{print $$NF}')
qemubg: all
	mkfifo $(PIPE).in $(PIPE).out
	$(UDK_DBG_EXEC) 2>&1 &
	$(UTILS)/monitor_qemu.sh > /dev/null 2>&1 & disown -h
	$(QEMU) $(QEMU_FLAGS_BDBG) -serial pipe:$(PIPE)

diskmu:
	@mkdir -p $(IMG_MDIR)
	@if ! sudo mount $(IMG_OUTPUT) $(IMG_MDIR) > /dev/null 2>&1;then \
		sudo umount $(IMG_MDIR) \
		;rm -rf $(IMG_MDIR) \
		;echo "Umounted the disk and removed the dir of the disk." \
	;else \
		echo "Created an dir for the disk and mounted the disk." \
	;fi

clean:
	make -C boot clean
	make -C kernel clean
	find . -iname "*.status_log" | xargs rm -f
	find . -iname "*.status_lock" | xargs rm -f
	rm -rf $(OUTPUT)


.PHONY: all _boot boot kernel app image \
		qemu qemug qemubg \
		diskmu clean \
		compile_commands.json 

