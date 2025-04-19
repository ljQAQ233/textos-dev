#include <app/api.h>
#include <app/inet.h>
#include <stdio.h>

char tx_buf[] = "test data!";
char rx_buf[128];

int main(int argc, char const *argv[])
{
    sockaddr_in_t addr = {
        .family = AF_INET,
        .port = htons(8080),
        .addr = 0,
    };

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror(NULL);
        return 1;
    }

    if (bind(fd, (sockaddr_t *)&addr, sizeof(sockaddr_in_t)) < 0)
    {
        perror(NULL);
        return 1;
    }

    if (listen(fd, 1) < 0)
    {
        perror(NULL);
        return 1;
    }

    printf("Listen on %d\n", ntohs(addr.port));

    sockaddr_in_t in;
    size_t insz = sizeof(sockaddr_in_t);
    for (;;)
    {
        int conn = accept(fd, (sockaddr_t *)&in, &insz);
        if (conn < 0)
        {
            perror(NULL);
            return 1;
        }

        printf("Connection from %s\n", inet_ntoa(in.addr));
        if (send(conn, tx_buf, sizeof(tx_buf), 0) < 0)
            perror(NULL);
    }

    return 0;
}
