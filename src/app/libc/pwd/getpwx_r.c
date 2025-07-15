#include "pwd.h"
#include <errno.h>
#include <malloc.h>
#include <string.h>

int __getpw(uid_t uid, const char *name, struct passwd *pwd)
{
    FILE *fp = fopen("/etc/passwd", "r");
    if (!fp)
        return PW_ERR;

    char *ln = NULL;
    size_t sz = 0;
    for (;;)
    {
        int ret = __getpwent(pwd, &ln, &sz, fp);
        if (ret <= 0) {
            free(ln);
            fclose(fp);
            return ret;
        }
        if (name && !strcmp(pwd->pw_name, name) ||
           !name && pwd->pw_uid == uid)
           break;
    }
    free(ln);
    fclose(fp);
    return PW_OK;
}

/*
 * getpwiuid_t / getpwnam_t 不改变 errno, 而是返回错误码:
 * On  success, getpwnam_r() and getpwuid_r() return zero, and set *result to pwd. If no matching password record
 * was found, these functions return 0 and store NULL in *result.  In case of error, an error number  is  returned,
 * and NULL is stored in *result.
 */
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    int ret;
    int olderr = errno;
    errno = 0;
    *result = NULL;

    if (__getpw(uid, NULL, pwd) < 0)
        goto fail;
    if (__pwdup(pwd, buf, buflen) < 0)
        goto fail;
    *result = pwd;

fail:
    ret = errno;
    errno = olderr;
    return ret;
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    int ret;
    int olderr = errno;
    errno = 0;
    *result = NULL;

    if (__getpw(0, name, pwd) < 0)
        goto fail;
    if (__pwdup(pwd, buf, buflen) < 0)
        goto fail;
    *result = pwd;

fail:
    ret = errno;
    errno = olderr;
    return ret;
}