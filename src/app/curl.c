#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

char req[512];
char buf[4096];

static int myatoi(char *s)
{
    int x = 0;
    char ch = 0;
    while (ch < '0' || ch > '9')
        ch = *s++;
    while (ch >= '0' && ch <= '9') {
        x = x * 10 + (ch - '0');
        ch = *s++;
    }
    return x;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: curl http://ip[:port]/path\n");
        exit(1);
    }

    char ip[64], path[256] = "/", port[8] = "80";
    char *url = argv[1];

    if (strncmp(url, "http://", 7) != 0) {
        printf("only http supported\n");
        exit(1);
    }

    url += 7;

    char *p = strchr(url, '/');
    if (p) {
        strncpy(path, p, sizeof(path)-1);
        *p = 0;
    }

    char *q = strchr(url, ':');
    if (q) {
        *q = 0;
        strncpy(ip, url, sizeof(ip)-1);
        strncpy(port, q + 1, sizeof(port)-1);
    } else {
        strncpy(ip, url, sizeof(ip)-1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("curl");
        exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(myatoi(port)),
    };
    inet_aton(ip, &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("curl");
        exit(1);
    }

    sprintf(req,
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "User-Agent: curl/0.1\r\n"
        "\r\n", path, ip);
    send(fd, req, strlen(req), 0);

    int n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0)
        write(1, buf, n);

    close(fd);
    return 0;
}
