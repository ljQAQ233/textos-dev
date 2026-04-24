#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/cdefs.h>

void *__sc_redir = 0;

#ifdef __x86_64__
    #include "x86_64/arch.c"
#else
static void __arch_init_wrapper(void **tab) {};
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

    if (__is_linux) {
        // musl libc gives us the value 100
        __sysconfs[_SC_CLK_TCK] = 100;
        __arch_init_wrapper(&__sc_redir);
    }
}
