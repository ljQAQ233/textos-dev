include $(dir $(lastword $(MAKEFILE_LIST)))/cmd.mk

BUILD := $(BUILD)/$(notdir ${CURDIR})

TARG := $(addprefix $(BUILD)/,${TARG})

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${BUILD}/,${OBJS})

$(BUILD)/%.c.o: %.c
	$(call compile-obj,$<,$@)

ifeq ($(suffix $(TARG)),.o)
$(TARG): $(OBJS)
	$(call compile-merge,$^,$@)
	@touch .stamp
else
$(TARG): $(OBJS)
	$(call compile-ld,$^,$@,)
	@touch .stamp
endif

all: $(TARG)

.PHONY: all
