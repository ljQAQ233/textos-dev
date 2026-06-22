#include <stddef.h>

extern void __dlm_free(void *);

void free(void *ptr)
{
    __dlm_free(ptr);
}
