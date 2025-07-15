#ifndef _CRYPT_H
#define _CRYPT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

struct crypt_data
{
    int initialized;
    char __buf[256];
};

char *crypt(const char *__key, const char *__salt);
char *crypt_r(const char *__key, const char *__salt, struct crypt_data *__data);

__END_DECLS

#endif