#ifndef _DLFCN_H
#define _DLFCN_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define RTLD_LAZY     1
#define RTLD_NOW      2
#define RTLD_NOLOAD   4
#define RTLD_NODELETE 4096
#define RTLD_GLOBAL   256
#define RTLD_LOCAL    0

#define RTLD_NEXT    ((void *)-1)
#define RTLD_DEFAULT ((void *) 0)

#define RTLD_DI_LINKMAP 2

int   dlclose(void *__h);
char *dlerror();
void *dlopen(const char *__path, int __flags);
void *dlsym(void *restrict __h, const char *restrict __name);

__END_DECLS

#endif
