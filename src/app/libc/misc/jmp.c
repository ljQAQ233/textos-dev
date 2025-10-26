#include <jmp.h>

__asm__
(
    ".global setjmp\n"
    ".global _setjmp\n"
    "setjmp:\n"
    "_setjmp:\n"
    _ASM_SETJMP
);

__asm__
(
    ".global longjmp\n"
    ".global _longjmp\n"
    "longjmp:\n"
    "_longjmp:\n"
    _ASM_LONGJMP
);
