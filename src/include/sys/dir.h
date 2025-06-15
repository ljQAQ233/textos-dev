#ifndef	_SYS_DIR_H
#define	_SYS_DIR_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* textos-specified */

#define __NEED_size_t
#define __NEED_ssize_t
#include <bits/alltypes.h>

typedef struct dirent
{
    int idx;
    size_t siz;
    size_t len;
    char name[];
} dir_t;

ssize_t __readdir(int fd, void *buf, size_t mx);

int __seekdir(int fd, size_t *pos);

__END_DECLS

#endif
