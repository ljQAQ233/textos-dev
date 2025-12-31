#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int isatty(int fd)
{
    struct winsize wsz;
    int r = ioctl(fd, TIOCGWINSZ, &wsz);
    if (r == 0) return 1;
    if (errno != EBADF) errno = ENOTTY;
    return 0;
}
