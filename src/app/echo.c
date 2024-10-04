#include <app/api.h>

void main(int argc, char *argv[], char *envp[])
{
    char buf[128] = "hello world!\n";
    int fd = open("/config2.ini", O_CREAT | O_RDWR);
    if (write(fd, buf, 18) > 0)
        write(1, "write successfully!\n", -1);
    close(fd);
    while(1);
}
