MOD ?= unknown
INST_ALL =

B ?=
L ?=
R ?=

# B = host:mode
define mk
INST_ALL += $(ROOT)/$(1)/$(notdir $(2))
$(ROOT)/$(1)/$(notdir $(2)): $(2)
	install -D $(if $(3),-m $(3)) $$< $$@
endef

# R = host:image:mode (-m)
# R = host:image
define mk-r
INST_ALL += $(ROOT)/$(MOD)/$(2)
$(INST)/$(MOD)/$(2): $(1)
	install -D $(if $(3),-m $(3)) $< $@
endef

define split-w
$(eval _w1 := $(word 1,$(subst :, ,$(1))))\
$(eval _w2 := $(word 2,$(subst :, ,$(1))))\
$(eval _w3 := $(word 3,$(subst :, ,$(1))))
endef

$(foreach i,$(B),\
	$(call split-w,$(i))\
	$(eval $(call mk,bin,$(_w1),$(_w2))))
$(foreach i,$(L),\
	$(call split-w,$(i))\
	$(eval $(call mk,lib,$(_w1),$(_w2))))
$(foreach i,$(R),\
	$(call split-w,$(i))\
	$(eval $(call mk-r,$(_w1),$(_w2),$(_w3))))

install: $(INST_ALL)
.PHONY: install
