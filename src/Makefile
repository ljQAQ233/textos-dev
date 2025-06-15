ARCH   := X64

export ARCH

include config/main.mk
include config/image.mk

include config/udk.mk

all: include/bits/alltypes.h _boot kernel app image

include config/qemu.mk

boot:
	@make -C boot build

_boot:
	@make -C boot update

ovmf:
	@make -C boot TARGET=RELEASE ovmf

ovmf-debug:
	@make -C boot TARGET=DEBUG FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE" ovmf

ovmf-noopt:
	@make -C boot TARGET=NOOPT FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE -DSOURCE_DEBUG_ENABLE=TRUE" ovmf

kernel:
	@make -C kernel kernel

app:
	@make -C app app

$(IMG) : _boot kernel app
image: $(IMG)

compile_commands.json: clean
	@bear --output utils/compile_commands.json -- make clean all
	@utils/fix_clcmd.sh utils/compile_commands.json

include config/net.mk

qemu: all net
	$(QEMU) $(QEMU_FLAGS_RUN)

qemug: all net
	$(QEMU) $(QEMU_FLAGS_KDBG)

qemubg: export BSRC_DEBUG:=true
qemubg: export PIPE:=$(shell cat $(UDK_DBG_CFG) | grep -E "^[P ]ort" | awk '{print $$NF}')
qemubg: all net
	mkfifo $(PIPE).in $(PIPE).out
	$(UDK_DBG_EXEC) 2>&1 &
	$utils/monitor_qemu.sh > /dev/null 2>&1 & disown -h
	$(QEMU) $(QEMU_FLAGS_BDBG) -serial pipe:$(PIPE)

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

