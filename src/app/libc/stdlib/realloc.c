#include <stddef.h>

extern void *__dlm_realloc(void *, size_t);

void *realloc(void *ptr, size_t size)
{
    return __dlm_realloc(ptr, size);
}
