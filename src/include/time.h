#ifndef _TIME_H
#define _TIME_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_struct_timespec
#include <bits/null.h>
#include <bits/alltypes.h>

time_t time(time_t /* _Nullable */ *tp);

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

__END_DECLS

#endif
