#include <app/api.h>


struct incmd
{
    char  *cmd;
    main_t handler;
};

#define BUILTIN(name) { .cmd = #name, .handler = builtin_##name }

MAIN(builtin_echo)
{
    for (int i = 1 ; i < argc ; i++)
        write(STDOUT_FILENO, argv[i], -1);
    return 0;
}

struct incmd builtin[] = 
{
    BUILTIN(echo)
};

int main(int argc, char const *argv[], char const *envp[])
{
    return 0;
}
