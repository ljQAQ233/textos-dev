#include <sys/cdefs.h>

static void *dummy;
__weak_alias(dummy, __fini_array_start);
__weak_alias(dummy, __fini_array_end);
