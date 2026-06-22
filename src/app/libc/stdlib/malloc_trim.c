#include <stddef.h>

extern int __dlm_malloc_trim(size_t);

int malloc_trim(size_t pad)
{
    return __dlm_malloc_trim(pad);
}
