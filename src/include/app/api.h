#pragma once

// api

int fork();

int execve(char *path, char *const argv[], char *const envp[]);

void _exit(int stat);

int wait4(int pid, int *stat, int opt, void *rusage);

int wait(int *stat);

typedef struct dirent
{
    int idx;
    size_t siz;
    size_t len;
    char name[];
} dir_t;

#define O_ACCMODE 0003 // 访问模式掩码
#define O_RDONLY  00   // 只读
#define O_WRONLY  01   // 只写
#define O_RDWR    02   // 读写

#define O_CREAT  0400  // 创建
#define O_EXCL   02000 // 互斥创建
#define O_TRUNC  01000 // 截断
#define O_APPEND 0010  // 末尾追加
#define O_DIRECTORY 0200000

int open(char *path, int flgs);

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t write(int fd, const void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

ssize_t readdir(int fd, void *buf, size_t mx);

int close(int fd);

#define S_IFMT 0170000

#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFBLK  0060000
#define S_IFREG  0100000
#define S_IFIFO  0010000
#define S_IFLNK  0120000
#define S_IFSOCK 0140000

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

typedef struct stat
{
    int mode;
    long dev;
    size_t siz;
} stat_t;

int stat(char *path, stat_t *sb);

int dup(int fd);

int dup2(int old, int new);

int pipe(int fds[2]);

#define major(x) ((unsigned)((((x) >> 31 >> 1) & 0xfffff000) | (((x) >> 8) & 0xfff)))
#define minor(x) ((unsigned)((((x) >> 12) & 0xffffff00) | ((x) & 0xff)))

#define makedev(x, y) \
    ((((x) & 0xfffff000ULL) << 32) | (((x) & 0xfffULL) << 8) | (((y) & 0xffffff00ULL) << 12) | (((y) & 0xffULL)))

int mknod(char *path, int mode, long dev);

int mount(char *src, char *dst);

int chdir(char *path);

int mkdir(char *path, int mode);

int rmdir(char *path);

/* address family */
#define AF_UNSPEC 0
#define AF_INET   2
#define AF_PACKET 17
#define AF_MAX    45

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3
#define SOCK_PACKET 10

typedef struct sockaddr_t
{
    u16 family;
    char data[14];
} sockaddr_t;

typedef struct sockaddr_in_t
{
    u16 family;
    u16 port;
    u8 addr[4];
    u8 zero[8];
} sockaddr_in_t;

typedef struct
{
    void *base;
    size_t len;
} iovec_t;

typedef struct
{
    sockaddr_t *name;
    int namelen;
    iovec_t *iov;
    size_t iovlen;
} msghdr_t;

int socket(int domain, int type, int proto);

int bind(int fd, sockaddr_t *addr, size_t len);

int connect(int fd, sockaddr_t *addr, size_t len);

ssize_t sendmsg(int fd, msghdr_t *msg, u32 flags);
ssize_t recvmsg(int fd, msghdr_t *msg, u32 flags);

ssize_t sendto(int fd, void *buf, size_t len, int flags, sockaddr_t *dst, size_t dlen);
ssize_t recvfrom(int fd, void *buf, size_t len, int flags, sockaddr_t *src, size_t slen);

ssize_t send(int fd, void *buf, size_t len, int flags);
ssize_t recv(int fd, void *buf, size_t len, int flags);

