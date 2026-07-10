BUILD := $(BUILD)/$(notdir ${CURDIR})

# Lua v5.5.0 (dev source)
LUA_MAKE := \
  $(MAKE) -f makefile \
  CC=$(BUILD)/libc/cc \
  MYCFLAGS="-std=c99 -DLUA_USE_POSIX -g -O0" \
  MYLDFLAGS="-static" \
  MYLIBS="-ldl"

all:
	@cd lua && $(LUA_MAKE) all

test:
	cd lua/testes/libs && $(LUA_MAKE) -j
	cd lua/testes && ulimit -S -s 1900 && ../lua -e"_U=1" -W all.lua

.PHONY: all test
