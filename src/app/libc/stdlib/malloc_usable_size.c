#include <stddef.h>

extern size_t __dlm_malloc_usable_size(void *);

size_t malloc_usable_size(void *ptr)
{
    return __dlm_malloc_usable_size(ptr);
}
