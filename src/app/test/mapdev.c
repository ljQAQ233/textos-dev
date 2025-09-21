#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

int main()
{
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    size_t sz = 1 << 20;
    uint32_t *fb = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    for (int i = 0 ; i < sz / 4 ; i++) {
        fb[i] = 0x0066ccff;
    }
    return 0;
}