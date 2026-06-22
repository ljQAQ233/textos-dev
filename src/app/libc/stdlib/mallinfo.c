#include <malloc.h>

extern struct mallinfo __dlm_mallinfo(void);

struct mallinfo mallinfo(void)
{
    return __dlm_mallinfo();
}
