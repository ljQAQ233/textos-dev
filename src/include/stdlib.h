#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/null.h>

void _Exit(int status);

void exit(int status);

// environ
int setenv(const char *name, const char *value, int overwrite);
int putenv(char *str);
int unsetenv(const char *name);
char *getenv(const char *name);

__END_DECLS

#endif