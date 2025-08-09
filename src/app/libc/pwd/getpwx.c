#include <pwd.h>
#include <stddef.h>

struct passwd *getpwuid(uid_t uid)
{
    static struct passwd pwd;
    static char buf[4096];
    struct passwd *res = NULL;
    getpwuid_r(uid, &pwd, buf, sizeof(buf), &res);
    return res;
}

struct passwd *getpwnam(const char *name)
{
    static struct passwd pwd;
    static char buf[4096];
    struct passwd *res = NULL;
    getpwnam_r(name, &pwd, buf, sizeof(buf), &res);
    return res;
}
