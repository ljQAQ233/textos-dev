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
        .sin_port = htons(8080),
        .sin_addr = 0,
    };

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror(NULL);
        return 1;
    }

    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror(NULL);
        return 1;
    }

    if (listen(fd, 1) < 0)
    {
        perror(NULL);
        return 1;
    }

    printf("Listen on %d\n", ntohs(addr.sin_port));

    struct sockaddr_in in;
    socklen_t insz = sizeof(struct sockaddr_in);
    for (;;)
    {
        int conn = accept(fd, (struct sockaddr *)&in, &insz);
        if (conn < 0)
        {
            perror(NULL);
            return 1;
        }

        printf("Connection from %s\n", inet_ntoa(in.sin_addr.s_addr));
        if (send(conn, tx_buf, sizeof(tx_buf), 0) < 0)
            perror(NULL);
    }

    return 0;
}
