#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#include <bits/alltypes.h>

int gettimeofday(struct timeval *__tp, void *__tzp);

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct itimerval
{
    struct timeval it_interval;
    struct timeval it_value;
};

// HISTORT:
//   POSIX.1-2001, SVr4, 4.4BSD (this call first appeared in 4.2BSD).
//   POSIX.1-2008 marks getitimer() and setitimer() obsolete,
//   recommending the use of the POSIX timers API (timer_gettime(2),
//   timer_settime(2), etc.) instead.
// TODO x 3
int getitimer(int __which, struct itimerval *__val);
int setitimer(int __which, const struct itimerval *restrict __val,
              struct itimerval *restrict __oval);
int utimes(const char *__path, const struct timeval /* Nullable */ __times[2]);

#endif
