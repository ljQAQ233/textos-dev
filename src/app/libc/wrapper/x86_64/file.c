#include "../util.h"
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>

#define BUFSZ 4096

typedef struct linux_dirent
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
} ldir_t;

struct dirrec
{
    int idx;
    unsigned boff;
    unsigned bmax;
    char buf[BUFSZ];
};

static long getdents(unsigned int fd, ldir_t *dirp, unsigned int count)
{
    return __w_syscall(78, fd, dirp, count);
}

static ssize_t w_readdir(int fd, void *buf, size_t mx)
{
    struct dirrec *r = __w_getval(0, fd);
    if (!r) {
        r = malloc(sizeof(struct dirrec));
        r->idx = 0;
        r->boff = -1, r->bmax = 0;
        if (!r) return -1;
        if (!__w_setval(0, fd, r)) return -1;
    }

    ssize_t sz = 0;
    while (sz < mx) {
        if (r->boff >= r->bmax) {
            long ret = getdents(fd, (ldir_t *)r->buf, BUFSZ);
            if (ret <= 0) {
                r->boff = -1, r->bmax = 0;
                if (ret) errno = ret;
                return sz ? sz : -1;
            }
            r->bmax = ret;
            r->boff = 0;
        }
        ldir_t *ldir = (ldir_t *)(r->buf + r->boff);
        dir_t *dir = (dir_t *)(buf + sz);
        dir->idx = r->idx++;
        dir->ino = ldir->d_ino;
        dir->siz = strlen(ldir->d_name) + 1 + sizeof(dir_t);
        strcpy(dir->name, ldir->d_name);
        sz += dir->siz;
        r->boff += ldir->d_reclen;
    }
    return sz;
}

static int w_close(int fd)
{
    __w_delval(0, fd);
    return __w_syscall(3, fd);
}
