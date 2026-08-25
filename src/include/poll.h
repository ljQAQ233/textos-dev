#ifndef _POLL_H
#define _POLL_H

#include <bits/poll.h>

int poll(struct pollfd __fds[], nfds_t __nfds, int __timeout /* ms */);

#endif
