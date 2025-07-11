#include <dirent.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/dir.h>

DIR *opendir(char *dirname)
{
    int fd = open((char *)dirname, O_DIRECTORY);
    if (fd < 0)
        return NULL;
    return fdopendir(fd);
}

DIR *fdopendir(int fd)
{
    if (fd < 0)
        return NULL;

    DIR *s = malloc(sizeof(DIR));
    s->fd = fd;
    s->tell = 0;
    s->boff = __DIR_BUFSZ;
    return s;
}

int closedir(DIR *dirp)
{
    close(dirp->fd);
    free(dirp);
    return 0;
}

struct dirent *readdir(DIR *dirp)
{
    if (dirp->boff >= __DIR_BUFSZ) {
        if (__readdir(dirp->fd, dirp->buf, __DIR_BUFSZ) < 0)
            return NULL;
        dirp->boff = 0;
    }

    dir_t *dir = (dir_t *)dirp->buf;
    struct dirent *cur = &dirp->cur;
    cur->d_ino = dir->ino;
    cur->d_off = dir->idx;  // compact
    cur->d_reclen = sizeof(struct dirent);
    cur->d_type = dir->type;
    strcpy(cur->d_name, dir->name);
    dirp->boff += dir->len;
    return cur;
}

void rewinddir(DIR *dirp)
{
    seekdir(dirp, 0);
}

long telldir(DIR *dirp)
{
    return dirp->tell;
}

void seekdir(DIR *dirp, long loc)
{
    size_t pos = loc;
    __seekdir(dirp->fd, &pos);
}

/* non-posix */
int dirfd(DIR *dirp)
{
    return dirp->fd;
}
