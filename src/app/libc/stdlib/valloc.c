#include <stddef.h>

extern void *__dlm_valloc(size_t);

void *valloc(size_t size)
{
    return __dlm_valloc(size);
}
