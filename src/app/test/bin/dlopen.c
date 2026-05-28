#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char *argv[])
{
    void *dl = dlopen("add.so", RTLD_LAZY);
    if (!dl)
    {
        puts(dlerror());
        return 1;
    }
    int (*pp)(int, int) = dlsym(dl, "add");
    printf("res = %d\n", pp(1, 2));
    return 0;
}
