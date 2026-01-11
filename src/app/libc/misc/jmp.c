#define _POSIX_C_SOURCE
#include <signal.h>
#include <setjmp.h>

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

int sigsetjmp(sigjmp_buf env, int savesigs)
{
    if (savesigs) {
        env->saved = 1;
        sigprocmask(SIG_SETMASK, NULL, &env->sig);
    }
    return setjmp(env->buf);
}

_Noreturn void siglongjmp(sigjmp_buf env, int val)
{
    if (env->saved)
        sigprocmask(SIG_SETMASK, &env->sig, NULL);
    longjmp(env->buf, val);
}
