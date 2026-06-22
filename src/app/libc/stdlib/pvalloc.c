#include <stddef.h>

extern void *__dlm_pvalloc(size_t);

void *pvalloc(size_t size)
{
    return __dlm_pvalloc(size);
}
