/*
 * simple tcp test
 *   - 3-way handshake
 *       - `python test/tcp/tcp-basic.py`
 *           - listen on port 8080 of the host, connect and close immediately
 *   - send/recv
 *       - `python test/tcp/tcp-echo.py`
 *           - textos sends data to 192.168.2.1:8080 and the host sends it back
 *   - delay ack
 *       - `python test/tcp/tcp-dack.py`
 *           - you will see textos piggybacks the ACK on outgoing data segment when no timeout occurs
*/
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

char tx_buf[] = "test data!";
char rx_buf[128];

#define TEST_SEND 1
#define TEST_ECHO 1
#define TEST_DACK 1

int main(int argc, char const *argv[])
{
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
    };
    inet_aton("192.168.2.1", &addr.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror(NULL);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror(NULL);
        return 1;
    }

#if !TEST_DACK

#if TEST_SEND || TEST_ECHO
    if (send(fd, tx_buf, sizeof(tx_buf), 0) < 0)
    {
        perror(NULL);
        return 1;
    }
#endif

#if TEST_ECHO
    if (recv(fd, rx_buf, sizeof(rx_buf), 0) < 0)
    {
        perror(NULL);
        return 1;
    }

    printf("tcp received echo : %s\n", rx_buf);
#endif

#else
    if (recv(fd, rx_buf, sizeof(rx_buf), 0) < 0)
    {
        perror(NULL);
        return 1;
    }
    
    if (send(fd, tx_buf, sizeof(tx_buf), 0) < 0)
    {
        perror(NULL);
        return 1;
    }

#endif
    
    return 0;
}
