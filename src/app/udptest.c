/*
 * simple udp test
 * 
 * to write bytes to emu's port 1, you can ->
 *   $ nc -p 2333 -uv 192.168.2.2 1
 * in this case, we setup a socket at port 2333
*/
#include <app/api.h>
#include <stdio.h>

char tx_buf[] = "test data!";
char rx_buf[128];

u16 htons(u16 h)
{
    return ((h & 0xFF00) >> 8) | ((h & 0x00FF) << 8);
}

u16 ntohs(u16 h)
{
    return ((h & 0xFF00) >> 8) | ((h & 0x00FF) << 8);
}

int main(int argc, char const *argv[])
{
    sockaddr_in_t addr = {
        .family = AF_INET,
        .addr = { 192, 168, 2, 1 },
        .port = htons(2333),
    };

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // if (connect(fd, (void *)&addr, sizeof(addr)) < 0)
    // {
    //     perror("connect");
    //     return 1;
    // }

    if (sendto(fd, tx_buf, sizeof(tx_buf), 0, (void *)&addr, sizeof(addr)) < 0)
    {
        perror("sendto");
        return 1;
    }

    sockaddr_in_t src;
    if (recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (void *)&src, sizeof(src)) < 0)
    {
        perror("recvfrom");
        return 1;
    }

    printf("received from %d.%d.%d.%d port %d: %s\n",
      src.addr[0], src.addr[1], src.addr[2], src.addr[3],
      ntohs(src.port), rx_buf);
    
    return 0;
}
