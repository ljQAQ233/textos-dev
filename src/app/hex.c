#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define N 4096

char buf[N];

int szgroup = 2;
int szline = 16;

void dumpl(int pos, int siz)
{
    printf("%08x: ", pos);

    int num = (siz + szgroup - 1) / szgroup;
    int idx = 0;
    for (int i = 0 ; i < num ; i++)
    {
        for (int j = 0 ; j < szgroup && idx < siz ; j++)
            printf("%02x", (unsigned char)buf[idx++]);
        if (i != num - 1)
            printf(" ");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int fd, siz, pos = 0;
    if (argc == 1)
        fd = 0;
    else
        fd = open(argv[1], O_RDONLY);

    while ((siz = read(fd, buf, szline)) > 0)
    {
        dumpl(pos, siz);
        pos += siz;
    }

    if (fd != 1)
        close(fd);
    return 0;
}
