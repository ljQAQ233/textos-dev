#ifndef _SYS_REOURCE
#define _SYS_REOURCE

#include <bits/rusage.h>

int getrusage(int who, struct rusage *ru);

#endif
