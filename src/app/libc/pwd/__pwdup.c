#include "pwd.h"
#include <errno.h>
#include <string.h>

int __pwdup(struct passwd *pwd, char *buf, size_t len)
{
    char **p[] = {
        &pwd->pw_name,
        &pwd->pw_passwd,
        &pwd->pw_gecos,
        &pwd->pw_dir,
        &pwd->pw_shell
    };
    int np = sizeof(p) / sizeof(char **);
    int pl[np];
    size_t bl = 0;
    for (int i = 0 ; i < np ; i++)
        bl += pl[i] = strlen(*p[i]) + 1;
    if (bl > len) {
        /*
         * 内部函数设置 errno, 而不是返回错误码
         */
        errno = ERANGE;
        return PW_ERR;
    }

    char *dst = buf;
    for (int i = 0 ; i < np ; i++) {
        *p[i] = memcpy(dst, *p[i], pl[i]);
        dst += pl[i];
    }
    return 0;
}