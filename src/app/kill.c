#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/*
 * kill -sig pid
 */

static int atoi(char *s)
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

static int help()
{
    dprintf(STDERR_FILENO, "usage : kill [-sig] pid\n");
    return 1;
}

int main(int argc, char *argv[])
{
    int pid;
    int sig = SIGTERM;
    if (argc == 2) {
        pid = atoi(argv[1]);
    } else if (argc == 3) {
        if (argv[1][0] != '-')
            return help();
        sig = atoi(argv[1]+1);
    } else {
        return help();
    }

    if (kill(pid, sig) < 0)
    {
        perror("kill");
        return 1;
    }
    return 0;
}
