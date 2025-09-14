SRCS := $(shell find . -type f -name '*.c')
BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})
CFLAGS += -fPIC

OBJS := $(addsuffix .o,${SRCS})
OBJS := $(addprefix ${BUILD}/,${OBJS})

all: \
	$(BUILD)/libc.a \
	$(BUILD)/libc.so

$(BUILD)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "\033[032m   CC  \033[0m $<"

$(BUILD)/libc.o: $(OBJS)
	@$(LD) $(LDFLAGS) $^ -r -o $@
	@touch .stamp
	@echo -e "\033[032m   LD  \033[0m link -> $@"

$(BUILD)/libc.so: $(BUILD)/libc.o
	@$(CC) $^ -o $@ -shared -fPIC
	@echo -e "\033[032m   CC  \033[0m $@"

$(BUILD)/libc.a: $(OBJS)
	@mkdir -p $(@D)
	@ar rcs $@ $^
	@echo -e "\033[032m   AR  \033[0m $@"
