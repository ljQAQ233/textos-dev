#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int putchar(int c)
{
    write(STDOUT_FILENO, &c, 1);
}

int puts(char *s)
{
    int len = strlen(s);
    write(STDOUT_FILENO, s, len);
}

void perror(const char *s)
{
    char *err = strerror(errno);
    char *spl = ": ";
    if (!s || !s[0])
    {
        s = "";
        spl = "";
    }
    dprintf(STDERR_FILENO, "%s%s%s\n", s, spl, err);
}
