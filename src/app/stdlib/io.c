#include <app/api.h>
#include <stdio.h>
#include <string.h>

// nobuffer

int putchar(int c)
{
    write(STDOUT_FILENO, &c, 1);
}

int puts(char *s)
{
    int len = strlen(s);
    write(STDOUT_FILENO, s, len);
}

