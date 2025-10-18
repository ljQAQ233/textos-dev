ARCH   := X64

export ARCH

include config/main.mk
include config/image.mk

all: pre boot kernel app image
pre: include/bits/alltypes.h

MODS := kernel boot app

# ovmf
ovmf:
	@make -C boot TARGET=RELEASE ovmf

ovmf-debug:
	@make -C boot TARGET=DEBUG FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE" ovmf

ovmf-noopt:
	@make -C boot TARGET=NOOPT FLAGS="-DDEBUG_ON_SERIAL_PORT=TRUE -DSOURCE_DEBUG_ENABLE=TRUE" ovmf

# image maker
image: $(IMG)

# QEMU commands
#   - qemu - run
#   - qemug - kernel debug
#   - qemubg - uefi debug (udk)
include config/net.mk
include config/udk.mk
include config/qemu.mk

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

# toolkits
compile_commands.json: clean
	@bear --output utils/compile_commands.json -- make clean all
	@utils/fix_clcmd.sh utils/compile_commands.json

clean:
	make -C boot clean
	make -C kernel clean
	find . -iname "*.status_log" | xargs rm -f
	find . -iname "*.status_lock" | xargs rm -f
	rm -rf $(OUTPUT)

.PHONY: all pre image qemu qemug qemubg \
		clean compile_commands.json $(MODS)

# special target : if the MAKECMDGOALS starts with those targets defined in MODS
# then it will be seen as calling the submodules and the arguments after it will
# be passed to the next level makefile. e.g. make -C src app clean-elfs
# a point worth attention is that although we pass them to the next level, these
# goals will be also executed after the submodule returns, which leads to an error
# thrown by this makefile, so %:: provided by GNU is useful to make it shut up :)
ifneq (,$(filter ${MODS},$(firstword ${MAKECMDGOALS})))
%::
	@true
$(MODS):
	$(MAKE) -C $@ $(filter-out $@,$(MAKECMDGOALS))
else
$(MODS):
	$(MAKE) -C $@
endif
