#include <app/api.h>

#define N 4096

char buf[N];

int main(int argc, char *argv[])
{
    int fd, siz;
    fd = open(argv[1], O_RDONLY);
    while ((siz = read(fd, buf, N)) > 0)
    {
        write(1, buf, siz);
    }
    close(fd);
    return 0;
}


