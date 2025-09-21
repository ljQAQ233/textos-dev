#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>

void handler(int sig)
{
    printf("sig %d received\n", sig);
}

int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid == 0) {
        signal(SIGTERM, handler);
        printf("registered\n");
        while (1) {
            syscall(SYS_yield);
        }
    } else {
        syscall(SYS_yield); // delay
        syscall(SYS_yield);
        if (kill(pid, SIGTERM) < 0)
        {
            perror("kill");
            return 1;
        }
        printf("signaled\n");
    }
    return 0;
}