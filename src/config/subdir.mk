# subdir.mk — '@' syntax routing for submodules
#   make -C src app/lua@test      -> cd into app/lua, runs 'test'
#   make -C src app@clean         -> cd into app, runs 'clean'
#   make -C src @compile_commands -> runs target at current level
#
# Include this in every Makefile that should act as a routing node.
# Normal builds (not include @ in MAKECMDGOALS) are unaffected.
# Set _SUBDIR_PRE := some-target before the include to add a prerequisite
# to the generated routing target.

_T := $(firstword $(foreach w,$(MAKECMDGOALS),$(if $(findstring @,$w),$w)))
ifneq (,$(_T))
_ST := $(patsubst @%,%,$(_T))

ifeq ($(_ST),$(_T))
  _P := $(firstword $(subst @, ,$(_T)))
  _G := $(word 2,$(subst @, ,$(_T)))
else
  _P :=
  _G := $(_ST)
endif

ifeq ($(_P),)
$(_T): $(_SUBDIR_PRE)
	$(MAKE) $(_G)
else
  _F := $(firstword $(subst /, ,$(_P)))
  _R := $(patsubst $(_F)/%,%,$(_P))
  ifeq ($(_F),$(_R))
    _PASS := $(_G)
  else
    _PASS := $(_R)@$(_G)
  endif
$(_T): $(_SUBDIR_PRE)
	$(MAKE) -C $(_F) $(_PASS)
endif
endif

# help: list targets at this level
help:
	@$(MAKE) -qp 2>/dev/null | grep -E '^[a-zA-Z_][a-zA-Z0-9_.%-]*:' \
		| sed 's/:.*//' | grep -v Makefile | sort -u | sed 's/^/  /'
.PHONY: help
