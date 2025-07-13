#ifndef __FILE_H__
#define __FILE_H__

#include <textos/fs.h>
#include <textos/klib/list.h>

#define S_PIPE_R 0x01  // rx
#define S_PIPE_W 0x02  // tx

typedef struct
{
    size_t offset;
    node_t *node;
    dirctx_t *dirctx;
    int flgs;
    int fdfl;
    int spec;
    int refer;
} file_t;

int fd_get(int min);
int file_get(int *new, file_t **file, int min);

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

typedef struct
{
    int idx;
    int type;
    ino_t ino;
    size_t siz;
    size_t len;
    char name[];
} dir_t;

#define S_IFMT 0170000

#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFBLK  0060000
#define S_IFREG  0100000
#define S_IFIFO  0010000
#define S_IFLNK  0120000
#define S_IFSOCK 0140000

// 特殊权限位 (特殊模式)
#define S_ISUID 04000  // 设置用户 ID 位(Set-user-ID)执行文件时切换为文件所有者的权限
#define S_ISGID 02000  // 设置组 ID 位(Set-group-ID)执行文件时切换为文件所属组的权限
#define S_ISVTX 01000  // 粘着位(Sticky bit)，通常用于目录中防止非所有者删除文件(如 /tmp)

// 所有者权限 (User)
#define S_IRUSR 0400   // 所有者可读(r)
#define S_IWUSR 0200   // 所有者可写(w)
#define S_IXUSR 0100   // 所有者可执行(x)
#define S_IRWXU 0700   // 所有者读/写/执行权限的组合(rwx)

// 群组权限 (Group)
#define S_IRGRP 0040   // 所属组可读(r)
#define S_IWGRP 0020   // 所属组可写(w)
#define S_IXGRP 0010   // 所属组可执行(x)
#define S_IRWXG 0070   // 所属组读/写/执行权限的组合(rwx)

// 其他用户权限 (Others)
#define S_IROTH 0004   // 其他用户可读(r)
#define S_IWOTH 0002   // 其他用户可写(w)
#define S_IXOTH 0001   // 其他用户可执行(x)
#define S_IRWXO 0007   // 其他用户读/写/执行权限的组合(rwx)

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

typedef struct stat
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
} stat_t;

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAXDEF_FILENO 3

extern file_t sysfile[MAXDEF_FILENO];

#define O_ACCMODE 0003 // 访问模式掩码
#define O_RDONLY  00   // 只读
#define O_WRONLY  01   // 只写
#define O_RDWR    02   // 读写

#define O_CREAT  0100  // 创建
#define O_EXCL   0200  // 互斥创建
#define O_TRUNC  01000 // 截断
#define O_APPEND 02000 // 末尾追加
#define O_DIRECTORY 0200000

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

typedef struct flock
{
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
} flock_t;

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4

#define F_GETLK  5
#define F_SETLK  6
#define F_SETLKW 7

#define F_SETOWN 8
#define F_GETOWN 9
#define F_SETSIG 10
#define F_GETSIG 11

#define FD_CLOEXEC 1

unsigned dir_get_type(mode_t mode);
bool dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type);
bool dir_emit_node(dirctx_t *ctx, node_t *chd);
bool dir_emit_dot(dirctx_t *ctx);
bool dir_emit_dotdot(dirctx_t *ctx);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#include <textos/ioctl.h>

#endif
