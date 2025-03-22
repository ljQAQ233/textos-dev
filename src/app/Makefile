SHELL  := bash

include ../config/app.mk

SRCS := $(wildcard *.c)

EXEC := $(SRCS:.c=.elf)
EXEC := $(addprefix ${APP_OUTPUT}/,${EXEC})

LIBSRC := \
  base/asm.c \
  base/crt0.c \
  stdlib/io.c \
  stdlib/error.c \
  stdlib/string.c \
  stdlib/printf.c \
  stdlib/malloc.c \
  stdlib/assert.c \
  stdlib/math.c \
  stdlib/inet.c

LIBOBJ := $(LIBSRC:.c=.o)
LIBOBJ := $(addprefix ${APP_OUTPUT}/,${LIBOBJ})

.PHONY: app
app: $(LIBC) $(APP_OUTPUT) $(EXEC)

$(APP_OUTPUT):
	@mkdir -p $(APP_OUTPUT)
	@echo -e "\033[032m   MKD \033[0m Created directory $@"

.PHONY: clean
clean:
	rm -rf $(APP_OUTPUT)

$(LIBC): $(LIBOBJ)
	@$(LD) $(LDFLAGS) $^ -r -o $@
	@echo -e "\033[032m   LD  \033[0m Linked all objects -> $@"

# Make rules and printing formats

$(APP_OUTPUT)/%.elf: %.c $(LIBC)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< $(LIBC) -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

$(APP_OUTPUT)/%.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"
