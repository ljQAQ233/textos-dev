#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

long syscall(int num, ...);

#if defined(__x86_64__)
#define __NR_statx 332
#else
#error "not supported arch"
#endif

#define STATX_TYPE 1U
#define STATX_MODE 2U
#define STATX_NLINK 4U
#define STATX_UID 8U
#define STATX_GID 0x10U
#define STATX_ATIME 0x20U
#define STATX_MTIME 0x40U
#define STATX_CTIME 0x80U
#define STATX_INO 0x100U
#define STATX_SIZE 0x200U
#define STATX_BLOCKS 0x400U
#define STATX_BASIC_STATS 0x7ffU
#define STATX_BTIME 0x800U
#define STATX_ALL 0xfffU

struct statx_timestamp
{
    int64_t tv_sec;
    uint32_t tv_nsec, __pad;
};

struct statx
{
    uint32_t stx_mask;
    uint32_t stx_blksize;
    uint64_t stx_attributes;
    uint32_t stx_nlink;
    uint32_t stx_uid;
    uint32_t stx_gid;
    uint16_t stx_mode;
    uint16_t __pad0[1];
    uint64_t stx_ino;
    uint64_t stx_size;
    uint64_t stx_blocks;
    uint64_t stx_attributes_mask;
    struct statx_timestamp stx_atime;
    struct statx_timestamp stx_btime;
    struct statx_timestamp stx_ctime;
    struct statx_timestamp stx_mtime;
    uint32_t stx_rdev_major;
    uint32_t stx_rdev_minor;
    uint32_t stx_dev_major;
    uint32_t stx_dev_minor;
    uint64_t __pad1[14];
};

#define AT_EMPTY_PATH 0x1000
#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100

static int convert(const struct statx *stx, struct stat *st)
{
    memset(st, 0, sizeof(struct stat));
    st->st_dev = makedev(stx->stx_dev_major, stx->stx_dev_minor);
    st->st_ino = stx->stx_ino;
    st->st_nlink = stx->stx_nlink;
    st->st_mode = stx->stx_mode;
    st->st_uid = stx->stx_uid;
    st->st_gid = stx->stx_gid;
    st->st_rdev = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
    st->st_size = stx->stx_size;
    st->st_blksize = stx->stx_blksize;
    st->st_blocks = stx->stx_blocks;
    st->st_atime = stx->stx_atime.tv_sec;
    st->st_mtime = stx->stx_mtime.tv_sec;
    st->st_ctime = stx->stx_ctime.tv_sec;
    return 0;
}

static int statx(int dirfd, const char *restrict pathname, int flags,
            unsigned int mask, struct statx *restrict statxbuf)
{
    return syscall(__NR_statx, dirfd, pathname, flags, mask, statxbuf);
}

int __libc_linux_stat(const char *path, struct stat *sb)
{
    struct statx stx;
    int ret = statx(AT_FDCWD, path, 0, STATX_BASIC_STATS, &stx);
    if (ret != 0)
        return ret;
    return convert(&stx, sb);
}

int __libc_linux_fstat(int fd, struct stat *sb)
{
    struct statx stx;
    int ret = statx(fd, "", AT_EMPTY_PATH, STATX_BASIC_STATS, &stx);
    if (ret != 0)
        return ret;
    return convert(&stx, sb);
}

int __libc_linux_lstat(const char *path, struct stat *sb)
{
    struct statx stx;
    int ret = statx(AT_FDCWD, path, AT_SYMLINK_NOFOLLOW, 
                   STATX_BASIC_STATS, &stx);
    if (ret != 0)
        return ret;
    return convert(&stx, sb);
}
