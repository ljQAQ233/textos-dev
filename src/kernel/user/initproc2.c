/**
 * @brief handle everything ready to switch to userspace, before jumping
 * into user space, and then hand privileges to /bin/init
 */
#include <cpu.h>
#include <textos/boot.h>
#include <textos/cmdline.h>
#include <textos/errno.h>
#include <textos/file.h>
#include <textos/mm/vmm.h>
#include <textos/panic.h>
#include <textos/printk.h>
#include <textos/syscall.h>
#include <textos/task.h>

extern RETVAL(int) sys_execve(char *, char **, char **);
extern RETVAL(int) sys_open(char *, int);
extern RETVAL(int) sys_dup2(int, int);
extern RETVAL(int) sys_close(int);

__CMDLINE_USED("init");
#define M 4096

static char *get_custom_init(char s[M])
{
    size_t n = M;
    int ret = cmdline_get_buf("init", s, &n);
    if (ret < 0) {
        if (ret == -ERANGE) {
            printk("custom init path out of range\n", s);
        }
        return NULL;
    }
    printk("custom init is '%s'\n", s);
    return s;
}

static void run_init(char *init)
{
    char *argv[] = {
        init,
        NULL,
    };
    char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/bin",
    };
    sys_execve(argv[0], argv, envp);
}

extern void initproc_mnt();
extern void initproc_nod();

void initproc2()
{
    int fd;
    char *custom_init;
    char custom_init_buf[M];

    initproc_mnt();
    initproc_nod();
    vfs_listnode(0);
    task_current()->umask = 0022;

    fd = sys_open("/dev/console", O_RDWR);
    if (fd < 0) {
        fd = sys_open("/dev/null", O_RDWR);
        if (fd < 0) PANIC("what did you do to /dev/null? - %d\n", fd);
    }
    assert(fd == 0);
    assert(sys_dup2(fd, 1) == 1);
    assert(sys_dup2(fd, 2) == 2);

    custom_init = get_custom_init(custom_init_buf);
    task_fade_kernproc(task_current());

    if (custom_init) {
        run_init(custom_init);
    }

    run_init("/bin/init");
    PANIC("init exiting...\n");
}
