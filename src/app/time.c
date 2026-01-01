#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define UNIT "\x1b[36ms\x1b[0m\n"

pid_t pid;

void redir_sig(int sig)
{
    printf("sig %d receiced.\n", sig);
    if (pid > 0)
        kill(pid, sig);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, redir_sig);

    pid = fork();
    if (pid == 0) {
        execvp(argv[1], (char **)&argv[1]);
        perror("execvp");
        return 1;
    } else {
        int status;
        struct rusage ru;
        if (wait4(-1, &status, 0, &ru) < 0)
            perror("wait4");
        // calc total
        useconds_t scale = 1000 * 1000;
        time_t total_sec = ru.ru_utime.tv_sec + ru.ru_stime.tv_sec;
        suseconds_t total_usec = ru.ru_utime.tv_usec + ru.ru_utime.tv_usec;
        total_sec += total_usec / scale;
        total_usec %= scale;

        printf("total  %ld.%06ld" UNIT, total_sec, total_usec);
        printf("sys  %ld.%06ld" UNIT, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
        printf("user  %ld.%06ld" UNIT, ru.ru_utime.tv_sec, ru.ru_utime.tv_usec);
    }
    return 0;
}
