#include <app/sys.h>
#include <app/api.h>

char buf[64];

void _start()
{
    char *argv[] = {
        "/config.ini",
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };

    int p[2];
    pipe(p);

    int pid = fork();
    if (pid == 0) {
        dup2(p[1], 1);
        execve("/cat.elf", argv, envp);
    } else {
        int siz = read(p[0], buf, sizeof(buf));
        write(1, buf, siz);
        while(1);
    }

    write(1, "execve failed!\n", 17);
    while(1);
}
