#include <fcntl.h>
#include <unistd.h>

#define N 4096
char buf[N];

int fcat(int fd)
{
    size_t n;
    while ((n = read(fd, buf, N)) > 0)
        write(1, buf, n);
    return 0;
}

int cat(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd < 0)
        return 1;
    fcat(fd);
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc == 1) {
        ret |= fcat(0);
    } else {
        for (int i = 1 ; i < argc ; i++)
            ret |= cat(argv[i]);
    }
    return ret;
}
