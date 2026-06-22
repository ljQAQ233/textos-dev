#include <stddef.h>

extern void *__dlm_memalign(size_t, size_t);

void *memalign(size_t alignment, size_t size)
{
    return __dlm_memalign(alignment, size);
}
