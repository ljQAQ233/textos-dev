#ifndef _ASSERT_H
#define _ASSERT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

_Noreturn void __assert_fail(const char *expr, const char *file, int line, const char *func);

#ifdef NDEBUG
#define	assert(x) (void)0
#else
#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__),0)))
#endif

__END_DECLS

#endif