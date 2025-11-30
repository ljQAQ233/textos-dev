#include <textos/errno.h>

int __noopt_inval()
{
    return -EINVAL;
}

int __noopt_perm()
{
    return -EPERM;
}
