/*
 * simple udp test
 * 
 * to write bytes to emu's port 1, you can ->
 *   $ nc -p 2333 -uv 192.168.2.2 1
 * in this case, we setup a socket at port 2333
*/
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

char tx_buf[] = "test data!";
char rx_buf[128];

int main(int argc, char const *argv[])
{
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2333),
    };
    inet_aton("192.168.2.1", &addr.sin_addr);

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

    struct sockaddr_in src;
    socklen_t socklen = sizeof(src);
    if (recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (void *)&src, &socklen) < 0)
    {
        perror("recvfrom");
        return 1;
    }

    printf("received from %s port %d: %s\n",
      inet_ntoa(src.sin_addr.s_addr),
      ntohs(src.sin_port), rx_buf);
    
    return 0;
}
