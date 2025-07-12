#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
    for (int i = 1 ; i < argc ; i++) {
        fputs(argv[i], stdout);
        if (i != argc - 1)
            fputs(" ", stdout);
    }
    fputc('\n', stdout);
    return 0;
}
