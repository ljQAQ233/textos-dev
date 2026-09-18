/**
 * @brief handle everything ready to switch to userspace, before jumping
 * into user space, and then hand privileges to /bin/init
 */
#include <cpu.h>
#include <textos/panic.h>

extern void sys_execve(char *, char **, char **);

static void run_init(char *init)
{
    char *argv[] = {
        init,
        NULL,
    };
    sys_execve(argv[0], argv, 0);
}

extern int close(int);

void initproc2()
{
    close(0);
    close(1);
    close(2);

    run_init("/bin/init");
    PANIC("init exiting...\n");
}
