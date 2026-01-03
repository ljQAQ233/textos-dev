#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_pid_t
#define __NEED_id_t
#include <bits/alltypes.h>

int wait(int *stat);

#include <sys/resource.h>

/* os-specified */
int wait4(pid_t pid, int *stat, int opt, void *rusage);

#define WEXITSTATUS(s)  (((s) & 0xff00) >> 8)
#define WTERMSIG(s)     ((s) & 0x7f)
#define WSTOPSIG(s)     WEXITSTATUS(s)
#define WCOREDUMP(s)    ((s) & 0x80)
#define WIFEXITED(s)    (!WTERMSIG(s))
#define WIFSTOPPED(s)   ((short)((((s) & 0xffff) * 0x10001U) >> 8) > 0x7f00)
#define WIFSIGNALED(s)  (((s) & 0xffff) - 1U < 0xffu)
#define WIFCONTINUED(s) ((s) == 0xffff)

__END_DECLS

#endif
