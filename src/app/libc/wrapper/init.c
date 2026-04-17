#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/cdefs.h>

#ifdef __x86_64__
    #include "x86_64/arch.c"
#else
void *__sc_redir[512];
static void __arch_init_wrapper() {};
#endif

int __is_linux;
extern long __sysconfs[];

void __init_wrapper()
{
    char b[1 << 9];
    struct utsname *u = (void *)b;
    if (uname(u) < 0) return;
    if (strcmp(u->sysname, "Linux") == 0)
        __is_linux = 1;
    else
        return;
    // musl libc gives us the value 100
    __sysconfs[_SC_CLK_TCK] = 100;
    __arch_init_wrapper();
}
