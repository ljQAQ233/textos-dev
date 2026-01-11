#ifndef _SETJMP_H
#define _SETJMP_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <jmp.h>

typedef __jmp_buf jmp_buf;
int setjmp(jmp_buf __env) __attribute__((returns_twice));
_Noreturn void longjmp(jmp_buf __env, int __val);

// simplest, rude unix _setjmp
#define _setjmp  setjmp
#define _longjmp longjmp

#if defined(_POSIX_C_SOURCE) || 1
    #include <bits/null.h>
    #define __NEED_sigset_t
    #include <bits/alltypes.h>
/*
 * jmpbuf with signal saved if required
 * many programs (e.g. bash) use the variable name to access the
 * address of this struct instead of `&name`, so [1] is used
 */
typedef struct
{
    int saved;
    jmp_buf buf;
    sigset_t sig;
} sigjmp_buf[1];

int sigsetjmp(sigjmp_buf __env, int __savesigs);
_Noreturn void siglongjmp(sigjmp_buf __env, int __val);
#endif

__END_DECLS

#endif
