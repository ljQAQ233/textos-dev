#ifndef	_FCNTL_H
#define	_FCNTL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_mode_t

#include <bits/alltypes.h>

int open(const char *__path, int __flgs, ...);

int fcntl(int __fd, int __cmd, ...);

#include <bits/fcntl.h>
#include <bits/perm.h>

__END_DECLS

#endif
