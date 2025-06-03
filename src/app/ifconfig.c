#include <app/api.h>
#include <app/inet.h>
#include <stdio.h>
#include <string.h>

#define ip4(x) (*((u32 *)x))

int ifconfig(char *name)
{
    int fd = socket(AF_INET, SOCK_RAW, 0);
    int ret = 0;
    ifreq_t ifr;
    strcpy(ifr.name, name);

    printf("%s:\n", name);
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
        ret = 1;
    else
        printf("  inet %s\n", inet_ntoa(ip4(ifr.addr)));

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
        ret = 1;
    else
        printf("  netmask %s\n", inet_ntoa(ip4(ifr.netmask)));

    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0)
        ret = 1;
    else
        printf("  broadcast %s\n", inet_ntoa(ip4(ifr.broadaddr)));

    if (ioctl(fd, SIOCGIFHWADDR, &ifr))
        ret = 1;
    else
        printf("  ether %02x:%02x:%02x:%02x:%02x:%02x\n",
            ifr.hwaddr[0], ifr.hwaddr[1], ifr.hwaddr[2],
            ifr.hwaddr[3], ifr.hwaddr[4], ifr.hwaddr[5]);
    
    return ret;
}

char b[4096];

int main(int argc, char const *argv[])
{
    int fd = open("/dev/net", O_DIRECTORY);
    if (fd < 0) {
        perror("ifconfig");
        return 1;
    }

    int ret = 0;
    int nr;
    while ((nr = __readdir(fd, b, sizeof(b))) > 0)
    {
        dir_t *p = (dir_t *)b;
        while (p < (dir_t *)(b + nr)) {
            if (p->name[0] != '.')
                ret |= ifconfig(p->name);
            p = (void *)p + p->siz;
        }
    }

    return ret;
}
