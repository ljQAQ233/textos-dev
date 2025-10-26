#ifndef	_SETJMP_H
#define	_SETJMP_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <jmp.h>

typedef __jmp_buf jmp_buf;
int setjmp(jmp_buf env) __attribute__((returns_twice));
_Noreturn void longjmp(jmp_buf env, int val);

__END_DECLS

#endif
