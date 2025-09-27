#include <dlfcn.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    void *dl1 = dlopen("libc.so", RTLD_LAZY);
    void *dl2 = dlopen("libc.so", RTLD_LAZY);
    assert(dl1 == dl2);
    return 0;
}
