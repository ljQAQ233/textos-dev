#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_pid_t
#define __NEED_id_t
#include <bits/alltypes.h>

int wait(int *stat);

/* os-specified */
int wait4(pid_t pid, int *stat, int opt, void *rusage);

__END_DECLS

#endif