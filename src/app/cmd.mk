# 1 - source files
# 2 - output path
# 3 - additional arguments
define compile-obj
  @mkdir -p $(dir $(2))
  @$(CC) $(CFLAGS) $(3) -c $(1) -o $(2)
  @echo -e "\033[032m   CC  \033[0m $(1)"
endef

# 1 - source files
# 2 - output path
# 3 - additional arguments
define compile-cc
  @mkdir -p $(dir $(2))
  @$(CC) $(CFLAGS) $(LDFLAGS) $(1) $(LDLIBS) $(3) -o $(2)
  @echo -e "\033[032m   CC  \033[0m $(1)"
endef

# 1 - source files
# 2 - output path
# 3 - additional arguments
define compile-ld
  @mkdir -p $(dir $(2))
  @$(CC) $(LDFLAGS) $(1) $(LDLIBS) $(3) -o $(2)
  @echo -e "\033[032m   LD  \033[0m $(1)"
endef

# 1 - input object files
# 2 - output path
# 3 - additional arguments
define compile-merge
  @mkdir -p $(dir $(2))
  @$(CC) $(LDFLAGS) $(1) $(3) -r -o $(2)
  @echo -e "\033[032m   LD  \033[0m $(2)"
endef

# 1 - input object files
# 2 - output path
define compile-ar
  @ar rcs $(2) $(1)
  @echo -e "\033[032m   AR  \033[0m $(2)"
endef

# 1 - input object files
# 2 - output path
# 3 - additional arguments
define compile-so
  @mkdir -p $(dir $(2))
  @$(CC) $(LDFLAGS) $(1) $(3) -shared -fPIC -o $(2)
  @echo -e "\033[032m   CC  \033[0m $(2)"
endef
