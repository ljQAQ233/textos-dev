#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"
PROG_NAME("cat");

#define N 4096
char buf[N];

int fcat(int fd)
{
    ssize_t n;
    while ((n = read(fd, buf, N)) > 0)
        if (write(1, buf, n) <= 0) break;
    return n < 0 ? -1 : 0;
}

int cat(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        error_x(file);
        return 1;
    }
    int ret = fcat(fd);
    if (ret) error_x(file);
    close(fd);
    return ret ? 1 : 0;
}

int main(int argc, char *argv[])
{
    int err = 0;
    if (argc == 1) {
        err |= fcat(0);
    } else {
        for (int i = 1; i < argc; i++)
            err |= cat(argv[i]);
    }
    return !!err;
}
