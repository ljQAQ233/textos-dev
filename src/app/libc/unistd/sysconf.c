#include <errno.h>
#include <unistd.h>

long __sysconfs[] = {
    [_SC_CLK_TCK] = 1000,
};

long sysconf(int name)
{
    long siz = sizeof(__sysconfs);
    long max = siz / sizeof(long);
    if (name <= 0 || name >= max) {
        errno = EINVAL;
        return -1;
    }
    return __sysconfs[name];
}
