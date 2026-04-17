#ifndef	_SYS_SYSCALL_H
#define	_SYS_SYSCALL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/syscall.h>

long syscall(long num, ...);

__END_DECLS

#endif
