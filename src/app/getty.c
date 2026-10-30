#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "util.h"

PROG_NAME("getty");
#define BUFMAX 128
char nodename[BUFMAX];

int getty(const char *path, const char *term)
{
    char *argv[] = {
        "/bin/login",
        NULL,
    };

    pid_t sid = setsid();
    if (sid < 0)
        if (getsid(0) != getpid()) goto err;

    // take over tty as our controlling terminal
    int fd = open(path, O_RDWR);
    if (fd < 0) goto err;
    if (fd != 0) goto err;

    dup2(fd, 1);
    dup2(fd, 2);
    printf("%s login: ", nodename);
    execv(argv[0], argv);
    write(1, "execve failed!\n", 17);
    while (1)
        pause();
err:
    error();
    return 1;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc != 2 && argc != 3) {
        pusage("port [term]");
        return 1;
    }

    const char *path = argv[1];
    const char *term = argc >= 3 ? argv[2] : "linux";
    
    if (gethostname(nodename, sizeof(nodename)) < 0)
        strcpy(nodename, "localhost");
    setenv("TERM", term, 1);
    return getty(path, term);
}
