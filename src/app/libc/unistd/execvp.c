#include <unistd.h>

extern char **__environ;

int execvp(const char *file, char *const argv[])
{
    return execvpe(file, argv, __environ);
}
