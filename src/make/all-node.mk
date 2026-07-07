MODS ?=

all-%:
	$(MAKE) -C $* all

all: $(MODS:%=all-%)
