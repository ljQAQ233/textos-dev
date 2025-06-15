#ifndef _MALLOC_H
#define _MALLOC_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#include <bits/alltypes.h>

void *malloc(size_t siz);

void free(void *ptr);

__END_DECLS

#endif