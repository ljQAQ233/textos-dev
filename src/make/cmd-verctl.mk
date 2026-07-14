# 1 - source dir
# 2 - patches dir
define git-patch
	$(eval _patched := $(patsubst $(2)/.%.patched,$(2)/%,$(wildcard $(2)/.*.patched)))
	$(foreach p,$(filter-out $(_patched),$(wildcard $(2)/*.patch)),
		@cd $(1) && git am $(realpath $(p))
		@touch $(abspath $(2)/.$(notdir $(p)).patched)
		@echo -e "\033[032m PATCH \033[0m" $(p)
	)
endef
