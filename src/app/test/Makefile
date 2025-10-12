SRCS := $(wildcard *.c */*.c)
BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})
TARS := $(SRCS:%.c=${BUILD}/%)

CC := $(APP_OUTPUT)/libc/cc

all: $(TARS)

$(BUILD)/%: %.c
	@mkdir -p $(@D)
	@$(CC) $(X) -fno-stack-protector $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

$(BUILD)/dlopen: X += -ldl
$(BUILD)/dluniq: X += -ldl
$(BUILD)/auxv: X += -static -g

.PHONY: all
