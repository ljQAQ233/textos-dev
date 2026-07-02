BUILD := $(APP_OUTPUT)/$(notdir ${CURDIR})

# Lua v5.5.0 (dev source)
LUA_MAKE := \
  $(MAKE) -f makefile \
  CC=$(APP_OUTPUT)/libc/cc \
  MYCFLAGS="-std=c99 -DLUA_USE_POSIX -g -O0" \
  MYLDFLAGS="-static" \
  MYLIBS="-ldl"

all:
	@mkdir -p $(@D)
	@cd lua && $(LUA_MAKE) clean
	@cd lua && $(LUA_MAKE) all
	@cp lua/lua $@

test:
	cd lua/testes/libs && $(LUA_MAKE) -j
	cd lua/testes && ulimit -S -s 1900 && ../lua -e"_U=1" -W all.lua

.PHONY: all test
