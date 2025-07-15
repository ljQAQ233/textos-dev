#include <pwd.h>
#include <stddef.h>

struct passwd *getpwuid(uid_t uid)
{
    static struct passwd pwd;
    struct passwd *res = NULL;
    getpwuid_r(uid, &pwd, NULL, 0, &res);
    return res;
}

struct passwd *getpwnam(const char *name)
{
    static struct passwd pwd;
    struct passwd *res = NULL;
    getpwnam_r(name, &pwd, NULL, 0, &res);
    return res;
}
