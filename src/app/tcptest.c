/*
 * simple tcp test
 *   - 3-way handshake
 *       - `python test/tcp/tcp-3whs.py`
 *           - listen on port 8080 of the host, connect and close immediately
*/
#include <app/api.h>
#include <app/inet.h>
#include <stdio.h>

char tx_buf[] = "test data!";

#define TEST_SEND 1

int main(int argc, char const *argv[])
{
    sockaddr_in_t addr = {
        .family = AF_INET,
        .port = htons(8080),
    };
    inet_aton("192.168.2.1", &addr.addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror(NULL);
        return 1;
    }

    if (connect(fd, (sockaddr_t *)&addr, sizeof(addr)) < 0)
    {
        perror(NULL);
        return 1;
    }

#if TEST_SEND
    if (send(fd, tx_buf, sizeof(tx_buf), 0) < 0)
    {
        perror(NULL);
        return 1;
    }
#endif
    
    return 0;
}
