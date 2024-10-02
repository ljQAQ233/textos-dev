#include <app/api.h>

void main(int argc, char *argv[], char *envp[])
{
    write(1, "echo\n", 5);
    for (int i = 0 ; i < argc ; i++) {
        write(1, argv[i], -1); // -1 不是正规写法
        write(1, "\n", -1);
    }
    while(1);
}
