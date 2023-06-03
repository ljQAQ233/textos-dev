CC   := gcc
NASM := nasm

NFLAGS := -f elf64 -g
CFLAGS := -std=c11 -g

%.out: %.s
	@$(NASM) $(NFLAGS) $< -o $@

%.out: %.asm
	@$(NASM) $(NFLAGS) $< -o $@

%.out: %.c
	@$(CC) $(CFLAGS) $< -o $@

%.dump: %.out
	@objdump -d -M intel $<

%.run: %.out
	@./$<

%.clean: %.out
	@rm ./$<
