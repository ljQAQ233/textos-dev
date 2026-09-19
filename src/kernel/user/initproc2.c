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

    sys_mkdir("/sysroot", 0755);
    sys_mount(BOOT_DEV_PATH_BOOT, "/sysroot");

    close(0);
    close(1);
    close(2);

    task_current()->istk = (addr_t)vmm_allocpages(istk_pages, PE_P | PE_RW) +
                           istk_pages * PAGE_SIZ;

    run_init("/sysroot/bin/init");
    PANIC("init exiting...\n");
}
