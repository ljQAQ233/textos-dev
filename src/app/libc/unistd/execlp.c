#include <stdarg.h>
#include <unistd.h>

int execlp(const char *file, const char *arg, ...)
{
    va_list ap;
    int argc = 1;
    va_start(ap, arg);
    while (va_arg(ap, char *))
        argc++;
    va_end(ap);

    int i;
    char *argv[argc + 1];
    va_start(ap, arg);
    argv[0] = (char *)arg;
    // copy remained and NULL
    for (i = 1; i <= argc; i++)
        argv[i] = va_arg(ap, char *);
    va_end(ap);
    return execvp(file, argv);
}
