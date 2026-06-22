#include <stddef.h>

extern int __dlm_posix_memalign(void **, size_t, size_t);

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    return __dlm_posix_memalign(memptr, alignment, size);
}
