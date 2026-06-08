#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

int system(const char *cmd)
{
    if (!cmd) return 1;

    struct sigaction sa_ignore, sa_int, sa_quit;
    sigset_t sigmask, sigmask_save;

    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;

    sigaction(SIGINT, &sa_ignore, &sa_int);
    sigaction(SIGQUIT, &sa_ignore, &sa_quit);

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sigmask, &sigmask_save);

    int ws;
    pid_t pid = fork();
    if (pid == 0) {
        sigaction(SIGINT, &sa_int, NULL);
        sigaction(SIGQUIT, &sa_quit, NULL);
        sigprocmask(SIG_SETMASK, &sigmask_save, NULL);
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }

    if (pid == -1) {
        ws = -1;
    } else {
        while (wait4(pid, &ws, 0, NULL) == -1) {
            if (errno != EINTR) {
                ws = -1;
                break;
            }
        }
    }

    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    sigprocmask(SIG_SETMASK, &sigmask_save, NULL);

    return ws;
}
