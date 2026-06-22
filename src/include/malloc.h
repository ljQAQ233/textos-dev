#ifndef _MALLOC_H
#define _MALLOC_H

#include <sys/cdefs.h>

#define __NEED_size_t
#include <bits/alltypes.h>

__BEGIN_DECLS

struct mallinfo
{
    size_t arena;
    size_t ordblks;
    size_t smblks;
    size_t hblks;
    size_t hblkhd;
    size_t usmblks;
    size_t fsmblks;
    size_t uordblks;
    size_t fordblks;
    size_t keepcost;
};

void *malloc(size_t __size);
void free(void *__ptr);
void *calloc(size_t __nmemb, size_t __size);
void *realloc(void *__ptr, size_t __size);

void *memalign(size_t __alignment, size_t __size);
int posix_memalign(void **__memptr, size_t __alignment, size_t __size);

void *valloc(size_t __size);
void *pvalloc(size_t __size);

size_t malloc_usable_size(void *__ptr);
int malloc_trim(size_t __pad);
void malloc_stats(void);

#define M_TRIM_THRESHOLD (-1)
#define M_GRANULARITY    (-2)
#define M_MMAP_THRESHOLD (-3)

int mallopt(int __param, int __value);
struct mallinfo mallinfo(void);

__END_DECLS

#endif
