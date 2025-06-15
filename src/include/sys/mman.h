#ifndef	_SYS_MMAN_H
#define	_SYS_MMAN_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_mode_t
#define __NEED_size_t
#define __NEED_off_t

#include <bits/alltypes.h>

#include <bits/mman.h>

void *mmap(void *addr, size_t len, int prot, int flgs, int fd, off_t off);

int mprotect(void *addr, size_t len, int prot);

int munmap(void *addr, size_t len);

__END_DECLS

#endif
