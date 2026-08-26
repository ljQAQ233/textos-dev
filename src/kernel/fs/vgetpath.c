#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/klib/string.h>

/*
 * get mount point's node if n is a mount point
 */
static node_t *getprt_mp(node_t *n)
{
    ASSERTK(!vfs_isaroot(n));
    n = n->parent;
    if (vfs_isaroot(n)) n = n->parent;
    return n;
}

int vfs_getpath(node_t *n, char *buf, size_t *size)
{
    if (n == NULL) {
        if (*size < 2) {
            *size = 2;
            return -ERANGE;
        }
        buf[0] = '/';
        buf[1] = 0;
        *size = 2;
        return 0;
    }
    if (vfs_isaroot(n)) n = n->parent;

    node_t *p;
    int len = 0;
    int toklen;
    for (p = n; p != p->parent; p = getprt_mp(p))
        len += strlen(p->name) + 1;

    if (*size < len + 1) {
        *size = len + 1;
        return -ERANGE;
    }

    char *end = buf + len;
    *end = 0;

    for (p = n; p != p->parent; p = getprt_mp(p)) {
        toklen = strlen(p->name);
        end -= toklen;
        memcpy(end, p->name, toklen);
        *(--end) = '/';
    }
    ASSERTK(end == buf);
    *size = len + 1;
    return 0;
}

