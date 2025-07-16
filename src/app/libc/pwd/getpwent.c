#include "pwd.h"
#include <string.h>

typedef unsigned long long ull;

/*
 * 如果成功, 返回解析的字符串, 失败则 NULL. 此时调用者应该放弃处理这一行!!!
 */
static inline char *pop(char **p, char **r)
{
    if (!*p)
        return NULL;
    *r = *p;
    *p = strchr(*r, ':');
    if (*p) {
        **p = 0;
        *p = *p + 1;
    }
    return *r;
}

static inline ull toull(char *p)
{
    ull r = 0;
    while (*p)
        r = r * 10 + *p++ - '0';
    return r;
}

#define ck(x) if (!x) return 0;

static int poppw(char *ln, struct passwd *p)
{
    char *_p = ln;
    char *_uid, *_gid;
    *strchr(_p, '\n') = 0;
    ck(pop(&_p, &p->pw_name));
    ck(pop(&_p, &p->pw_passwd));
    ck(pop(&_p, &_uid));
    ck(pop(&_p, &_gid));
    ck(pop(&_p, &p->pw_gecos));
    ck(pop(&_p, &p->pw_dir));
    ck(pop(&_p, &p->pw_shell));
    p->pw_uid = toull(_uid);
    p->pw_gid = toull(_gid);
    return PW_OK;
}

int __getpwent(struct passwd *pwd, char **ln, size_t *sz, FILE *fp)
{
    for (;;) {
        int ll = getline(ln, sz, fp);
        if (ll < 0)
        {
            if (ferror(fp))
                return PW_ERR;
            return PW_NOENT;
        }
        char *s = *ln;
        if (poppw(s, pwd))
            return PW_OK;
    }
}

static FILE *fp;
static char *ln;
static size_t sz;

void setpwent()
{
    if (!fp)
        fp = fopen("/etc/passwd", "r");
    else {
        // TODO
    }
}

void endpwent()
{
    if (fp)
        fclose(fp);
}

struct passwd *getpwent()
{
    if (!fp)
        setpwent();
    static struct passwd pwd;
    if (__getpwent(&pwd, &ln, &sz, fp) <= 0)
        return NULL;
    return &pwd;
}
