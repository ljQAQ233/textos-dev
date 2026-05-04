/**
 * @brief handle everything ready to switch to userspace, before jumping
 * into user space, and then hand privileges to /bin/init
 */
#include <cpu.h>
#include <textos/panic.h>
#include <bits/a-sc.h>
#include <bits/syscall.h>

static void run_init(char *init)
{
    /*
     * Now that we have a stack in user space, we can't call sys_execve
     * directly, which causes a unrecoverable pagefault after switch pagetable
     * (you can give it a shot). syscalls can switch to kernel stack first.
     */
    char *argv[] = {
        init,
        NULL,
    };
    __syscall(SYS_execve, argv[0], argv, 0, 0, 0, 0);
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
