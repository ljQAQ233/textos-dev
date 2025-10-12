#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
    char **env = envp;
    while (*env)
        puts(*env++);
    return 0;
}
