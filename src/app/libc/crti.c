#include <sys/cdefs.h>

asm(".section .init\n"
    ".global _init\n"
    "_init: push %rax");
asm(".section .fini\n"
    ".global _fini\n"
    "_fini: push %rax");
