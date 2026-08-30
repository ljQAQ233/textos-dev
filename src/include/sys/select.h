#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#include <bits/alltypes.h>
#include <bits/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds,
           struct timeval *timeout);

__END_DECLS

#endif
