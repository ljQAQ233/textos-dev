MODS ?=

install-%:
	$(MAKE) -C $* install

install: $(MODS:%=install-%)
