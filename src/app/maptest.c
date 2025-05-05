#include <app/api.h>
#include <stdio.h>
#include <string.h>

int main()
{
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    size_t sz = 1 << 20;
    u32 *fb = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    for (int i = 0 ; i < sz / 4 ; i++) {
        fb[i] = 0x0066ccff;
    }
    return 0;
}