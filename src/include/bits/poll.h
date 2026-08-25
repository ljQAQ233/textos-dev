typedef unsigned long nfds_t;

// POSIX
#define POLLIN     0x001 // 有普通或带外数据可读 (= POLLRDNORM | POLLRDBAND)
#define POLLPRI    0x002 // 有高优先级数据可读
#define POLLOUT    0x004 // 可写而不会阻塞
#define POLLERR    0x008 // 出错, 仅在 revents 中返回
#define POLLHUP    0x010 // 挂断, 仅在 revents 中返回
#define POLLNVAL   0x020 // fd 无效/未打开, 仅在 revents 中返回
#define POLLRDNORM 0x040 // 有普通数据可读 (POLLIN 的子集)
#define POLLRDBAND 0x080 // 有带外优先数据可读
#define POLLWRNORM 0x100 // 有普通数据可写 (POLLOUT 的子集)
#define POLLWRBAND 0x200 // 有带外优先数据可写

struct pollfd
{
    int fd;
    short events;
    short revents;
};
