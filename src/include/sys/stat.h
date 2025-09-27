#ifndef	_SYS_STAT_H
#define	_SYS_STAT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_dev_t
#define __NEED_ino_t
#define __NEED_mode_t
#define __NEED_nlink_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_time_t
#define __NEED_blksize_t
#define __NEED_blkcnt_t
// #define __NEED_struct_timespec

#include <bits/alltypes.h>

struct stat
{
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

// POSIX.1-2008
/*
  #define st_atime st_atim.tv_sec
  #define st_mtime st_mtim.tv_sec
  #define st_ctime st_ctim.tv_sec
*/

#include <bits/mode.h>
#include <bits/perm.h>

#define UTIME_NOW  0x3fffffff
#define UTIME_OMIT 0x3ffffffe

int stat(const char *path, struct stat *sb);
int lstat(const char *path, struct stat *sb);
int fstat(int fd, struct stat *sb);
int mkdir(const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);

__END_DECLS

#endif
