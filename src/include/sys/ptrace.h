#ifndef _SYS_PTRACE_H
#define _SYS_PTRACE_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/ptrace.h>

long ptrace(int req, ...);

__END_DECLS

#endif
