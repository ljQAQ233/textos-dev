#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int execvpe(const char *file, char *const argv[], char *const envp[])
{
    const char *path = getenv("PATH");
    size_t l = strlen(path);
    // treat it as an absolute path.
    if (strchr(file, '/'))
        return execve(file, argv, envp);

    size_t s1;
    size_t s2 = strlen(file);
    const char *p = path;
    const char *n = path;
    int acces = 0;
    for (;n; p = n + 1)
    {
        n = strchr(p, ':');
        s1 = n ? n - p : path + l - p;
        char b[s1 + 1 + s2 + 1];
        memcpy(b + 0, p, s1);
        b[s1] = '/';
        memcpy(b + 1 + s1, file, s2);
        b[s1 + 1 + s2] = '\0';

        execve(b, argv, envp);
        switch (errno)
        {
        case EACCES:
            acces = 1;
        case ENOENT:
        case ENOTDIR:
            break;
        default:
            return -1;
        }
    }
    if (acces)
        errno = EACCES;
    return -1;
}
