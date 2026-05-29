#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "unity/unity.h"

static volatile int sig_caught = 0;

static void handler(int sig)
{
    sig_caught = sig;
}

void sys_signal_catch(void)
{
    sig_caught = 0;
    signal(SIGTERM, handler);
    raise(SIGTERM);
    TEST_ASSERT_EQUAL_INT(SIGTERM, sig_caught);
}

void sys_signal_handler(void)
{
    pid_t pid = fork();
    if (pid == 0) {
        pause();
        _exit(0);
    }
    syscall(SYS_yield);
    syscall(SYS_yield);
    TEST_ASSERT_EQUAL_INT(0, kill(pid, SIGKILL));
    int wstatus;
    wait4(pid, &wstatus, 0, 0);
    TEST_ASSERT_TRUE(WIFSIGNALED(wstatus));
    TEST_ASSERT_EQUAL_INT(SIGKILL, WTERMSIG(wstatus));
}

void sys_signal_ignore(void)
{
    sig_caught = 0;
    signal(SIGTERM, SIG_IGN);
    raise(SIGTERM);
    TEST_ASSERT_EQUAL_INT(0, sig_caught);
}

void sys_signal_raise(void)
{
    sig_caught = 0;
    signal(SIGUSR1, handler);
    raise(SIGUSR1);
    TEST_ASSERT_EQUAL_INT(SIGUSR1, sig_caught);
}

//! register=sys_signal_catch
//! register=sys_signal_handler
//! register=sys_signal_ignore
//! register=sys_signal_raise
