#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_struct_iovec
#include <bits/alltypes.h>

ssize_t readv(int __fd, const struct iovec *__iov, int __iovcnt);
ssize_t writev(int __fd, const struct iovec *__iov, int __iovcnt);

__END_DECLS

#endif
