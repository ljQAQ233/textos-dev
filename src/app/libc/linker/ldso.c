/*
 * dynamic linker
 *   - static-linked with `libc.a` and make it minimum
 */
#include <stdio.h>
#include <dlfcn.h>
#define __NEED_linker
#include "linker.c"
#include "resolve.c"

static int help(const char *prog)
{
    dprintf(1, "try %s <program> instead\n", prog);
    return 1;
}


/*
 * TODO: catch dlfcn operations provided by extern libs to keep the linkmap only one in the memory
 * TODO: use auxv to check if this program is called by user directly or the operating system
 */

int main(int argc, const char *argv[], const char *envp[])
{
    // if (argc == 1)
    //     return help(argv[0]);

    /*
     * as a dynamic linker called by os.
     */
    struct dl *self = loadlib(argv[0], RTLD_LAZY);
    __dlself = self;
    if (!self)
    {
        dprintf(1, "%s\n", dlerror());
        return 1;
    }
    void *entry = lkprec(self, "_start");
    asm volatile(
        "mov %0, %%rsp\n"
        "jmp *%1\n"
        :
        : "r"(argv-1), "r"(entry)
        : "memory"
    );

    return 0;
}
