#ifndef _POLL_H
#define _POLL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/poll.h>

int poll(struct pollfd __fds[], nfds_t __nfds, int __timeout /* ms */);

__END_DECLS

#endif
