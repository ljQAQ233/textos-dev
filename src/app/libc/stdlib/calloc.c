#include <stddef.h>

extern void *__dlm_calloc(size_t, size_t);

void *calloc(size_t nmemb, size_t size)
{
    return __dlm_calloc(nmemb, size);
}
