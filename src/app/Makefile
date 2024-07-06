SHELL  := bash

include ../config/app.mk

SRCS := $(wildcard *.c)

EXEC := $(SRCS:.c=.elf)
EXEC := $(addprefix ${APP_OUTPUT}/,${EXEC})

.PHONY: app
app: $(APP_OUTPUT) $(EXEC)

$(APP_OUTPUT):
	@mkdir -p $(APP_OUTPUT)
	@echo -e "\033[032m   MKD \033[0m Created directory $@"

.PHONY: clean
clean:
	rm -rf $(APP_OUTPUT)

# Make rules and printing formats

$(APP_OUTPUT)/%.elf: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

