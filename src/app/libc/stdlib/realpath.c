#include <errno.h>
#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if 0
    #define log(x, ...) fprintf(stderr, x, ##__VA_ARGS__);
#else
    #define log(x, ...) ((void)0)
#endif

#define mempad(chrptr, chr) (*(char *)(chrptr) = (chr))

#define MAXSYMLINKS 40

// resolved_path is a ptr to a buffer with a size of at least PATH_MAX
char *realpath(const char *restrict path, char *restrict res)
{
    char *p;
    char abs[PATH_MAX * 2];
    char tmp[PATH_MAX];
    struct stat sb;
    ssize_t linksz;
    size_t res_i = 0;
    int allocated = 0;
    int symlinks = 0;

    if (!res) {
        res = malloc(PATH_MAX);
        if (!res) goto fail;
        allocated = 1;
    }
    if (path[0] != '/') {
        if (!(getcwd(tmp, sizeof(tmp)))) goto fail;
        size_t cwdlen = strlen(tmp);
        size_t pathlen = strlen(path);
        if (cwdlen + 1 + pathlen >= sizeof(abs)) {
            errno = ENAMETOOLONG;
            goto fail;
        }
        memcpy(abs, tmp, cwdlen);
        mempad(abs + cwdlen, '/');
        memcpy(abs + cwdlen + 1, path, pathlen + 1);
    } else {
        if (strlen(path) >= sizeof(abs)) {
            errno = ENAMETOOLONG;
            goto fail;
        }
        strcpy(abs, path);
    }
    p = abs;
    res[0] = '\0';

    for (;;) {
        while (*p == '/')
            p++;
        if (!*p) break;
        char *next_slash = strchrnul(p, '/');
        size_t sz = next_slash - p;
        if (sz == 1 && p[0] == '.') goto skip;
        if (sz == 2 && p[0] == '.' && p[1] == '.') {
            while (res_i > 0 && res[res_i - 1] != '/')
                res[--res_i] = '\0';
            if (res_i > 0) res[--res_i] = '\0';
            goto skip;
        }

        if (res_i + 1 + sz >= PATH_MAX) {
            errno = ENAMETOOLONG;
            goto fail;
        }
        mempad(res + res_i, '/');
        memcpy(res + res_i + 1, p, sz);
        mempad(res + res_i + 1 + sz, '\0');
        log("check %s\n", res);
        if (lstat(res, &sb) < 0) goto fail;
        if (S_ISLNK(sb.st_mode)) {
            if (++symlinks > MAXSYMLINKS) {
                errno = ELOOP;
                goto fail;
            }
            if ((linksz = readlink(res, tmp, sizeof(tmp))) < 0) goto fail;
            if ((size_t)linksz >= sizeof(tmp)) {
                errno = ENAMETOOLONG;
                goto fail;
            }
            tmp[linksz] = '\0';
            char *dst = p;
            if (tmp[0] == '/') {
                dst = abs;
                res[0] = '\0';
                res_i = 0;
            }
            size_t rem = strlen(next_slash);
            size_t off = dst - abs;
            if (off + (size_t)linksz + rem >= sizeof(abs)) {
                errno = ENAMETOOLONG;
                goto fail;
            }
            memmove(dst + linksz, next_slash, rem);
            memcpy(dst, tmp, linksz);
            mempad(dst + linksz + rem, '\0');
            p = dst;
            continue;
        }
        if (!S_ISDIR(sb.st_mode) && *next_slash != '\0') {
            // file cannot have its sub-files
            errno = ENOTDIR;
            goto fail;
        }
        res_i += 1 + sz;
    skip:
        if (*next_slash == 0) break;
        p = next_slash + 1;
    }

    if (!res_i) res[res_i++] = '/';
    res[res_i] = '\0';

    return res;

fail:
    if (allocated) free(res);
    return NULL;
}
