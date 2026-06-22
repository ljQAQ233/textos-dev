#include <stddef.h>

extern void *__dlm_malloc(size_t);

void *malloc(size_t size)
{
    return __dlm_malloc(size);
}
