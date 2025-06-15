#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/null.h>

void _Exit(int status);

void exit(int status);

__END_DECLS

#endif