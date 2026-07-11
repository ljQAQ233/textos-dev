R_BIN := $(patsubst %.c,%,$(wildcard bin/*.c))
R_SOLIB := $(patsubst %.c,%.so,$(wildcard solib/*.c))

R += $(BUILD)/runtest:test/runtest
R += $(foreach f,$(R_BIN),$(BUILD)/$(f):test/$(f))
R += $(foreach f,$(R_SOLIB),$(BUILD)/$(f):test/$(f))
