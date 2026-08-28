#include "sys/wait.h"
#include <poll.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

char buf[1024];

#define out(s) write(STDOUT_FILENO, s, sizeof(s))

int main()
{
    int p[2] = {-1, -1};
    if (pipe(p) < 0) goto err;

    int pid = fork();
    if (pid < 0) goto err;
    if (pid == 0) {
        close(p[1]);
        p[1] = -1;
        for (;;) {
            struct pollfd fds[1] = {
                [0] = {p[0], POLLIN},
            };
            int ret = poll(fds, 1, 2000);
            if (ret < 0) goto err;
            if (ret == 0) {
                out("timeout\n");
                continue;
            }
            ret = read(p[0], buf, sizeof(buf));
            if (ret < 0) goto err;
            if (ret == 0) break;
            write(STDOUT_FILENO, buf, ret);
        }
        _exit(0);
    }

    close(p[0]);
    p[0] = -1;
    for (;;) {
        int ret = read(STDIN_FILENO, buf, sizeof(buf));
        if (ret < 0) goto err;
        if (ret == 0) break;
        if (write(p[1], buf, ret) < 0) goto err;
    }

    return 0;
err:
    perror(NULL);
    if (p[0] >= 0) close(p[0]);
    if (p[1] >= 0) close(p[1]);
    if (pid > 0) waitpid(pid, NULL, 0);
    return 1;
}
