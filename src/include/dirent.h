#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_ino_t
#define __NEED_off_t
#include <bits/alltypes.h>

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

struct dirent
{
    ino_t          d_ino;       // Inode number (file serial number)
    off_t          d_off;       // Offset to the next dirent (not used!!!)
    unsigned short d_reclen;    // Length of this record
    unsigned char  d_type;      // File type (e.g., DT_REG, DT_DIR)
    char           d_name[256]; // Null-terminated filename
};

#define __DIR_BUFSZ 4096

typedef struct __dirstream
{
    int fd;
    off_t tell;
    unsigned boff;
    char buf[__DIR_BUFSZ];
    struct dirent cur;
} DIR;

DIR *opendir(char *dirname);
DIR *fdopendir(int fd);
int closedir(DIR *dirp);

struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, long loc);

/*
 * It leads to a link error
 */
__attribute__((deprecated("readdir_r is deprecated, use readdir instead")))
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

__END_DECLS

#endif