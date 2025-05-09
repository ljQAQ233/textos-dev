APP_OUTPUT := $(APP_OUTPUT)/$(notdir ${CURDIR})

TARG := $(addprefix $(APP_OUTPUT)/,${TARG})

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${APP_OUTPUT}/,${OBJS})

$(APP_OUTPUT)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

ifeq ($(suffix $(TARG)),.o)
$(TARG): $(OBJS)
	@$(LD) $(LDFLAGS) $^ -r -o $@
else ifeq ($(suffix $(TARG)),.elf)
$(TARG): $(OBJS)
	@$(LD) $(LDFLAGS) $^ -o $@
else
	$(warning "unsupported format - ${TARG}")
endif
