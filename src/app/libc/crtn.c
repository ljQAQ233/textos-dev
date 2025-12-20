#include <sys/cdefs.h>

asm(".section .init\n"
    "pop %rax \n ret");
asm(".section .fini\n"
    "pop %rax \n ret");
