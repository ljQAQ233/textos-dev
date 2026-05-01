#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define N 4096
char buf[N];

void sigint(int sig)
{
    puts("========");
    puts("signaled");
    puts("========");
}

void fcat(int fd)
{
    ssize_t n;
    while ((n = read(fd, buf, N)) > 0)
        write(STDOUT_FILENO, buf, n);
    if (n < 0) perror(NULL);
}

int main(int argc, char *argv[])
{
    assert(signal(SIGINT, sigint) >= 0);
    setvbuf(stdout, 0, _IONBF, 0); // sync with write

    puts("--- Part 1: signal() defaults to SA_RESTART ---");
    puts("Starting fcat, Ctrl+C will NOT return EINTR...\n");
    fcat(0);

    struct sigaction oact;
    assert(sigaction(SIGINT, 0, &oact) >= 0);
    oact.sa_handler = sigint;
    oact.sa_flags &= ~SA_RESTART;
    assert(sigaction(SIGINT, &oact, 0) >= 0);

    puts("--- Part 2: sigaction() clears SA_RESTART ---");
    puts("Starting fcat, Ctrl+C WILL return EINTR...\n");
    fcat(0);
    return 0;
}
