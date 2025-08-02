#pragma cone

typedef int (*tty_ioctl_t)(void *io, int req, void *argp);
typedef ssize_t (*tty_iosop_t)(void *io, char *s, size_t len);

typedef struct
{
    void *data;
    tty_ioctl_t ctl;
    tty_iosop_t in;
    tty_iosop_t out;
} tty_ios_t;
