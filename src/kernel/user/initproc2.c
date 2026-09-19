/**
 * @brief handle everything ready to switch to userspace, before jumping
 * into user space, and then hand privileges to /bin/init
 */
#include <cpu.h>
#include <textos/boot.h>
#include <textos/mm/vmm.h>
#include <textos/panic.h>
#include <textos/syscall.h>
#include <textos/task.h>

extern RETVAL(int) sys_execve(char *, char **, char **);
extern RETVAL(int) sys_mount(char *, char *);
extern RETVAL(int) sys_mkdir(char *, int);

static void run_init(char *init)
{
    char *argv[] = {
        init,
        NULL,
    };
    sys_execve(argv[0], argv, 0);
}

extern void initproc_mnt();
extern void initproc_nod();

extern int close(int);

void initproc2()
{
    initproc_mnt();
    initproc_nod();
    vfs_listnode(0);

    close(0);
    close(1);
    close(2);

    task_fade_kernproc(task_current());
    run_init("/bin/init");
    PANIC("init exiting...\n");
}
