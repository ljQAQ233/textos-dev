include $(dir $(lastword $(MAKEFILE_LIST)))/cmd.mk

APP_OUTPUT := $(APP_OUTPUT)/$(notdir ${CURDIR})

TARG := $(addprefix $(APP_OUTPUT)/,${TARG})

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${APP_OUTPUT}/,${OBJS})

$(APP_OUTPUT)/%.c.o: %.c
	$(call compile-obj,$<,$@)

ifeq ($(suffix $(TARG)),.o)
$(TARG): $(OBJS)
	$(call compile-merge,$^,$@)
	@touch .stamp
else
$(TARG): $(OBJS)
	$(call compile-ld,$^,$@,)
	@touch .stamp
	@mkdir -p $(ROOT)/bin
	@cp $@ $(ROOT)/bin
endif
