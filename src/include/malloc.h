#ifndef _MALLOC_H
#define _MALLOC_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#include <bits/alltypes.h>

void *malloc(size_t __siz);

void free(void *__ptr);

__END_DECLS

#endif