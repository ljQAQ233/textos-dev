#include <sys/cdefs.h>
void dummy() { };
__weak_alias(dummy, dlopen);
__weak_alias(dummy, dlclose);
__weak_alias(dummy, dlsym);
__weak_alias(dummy, dlerror);
