#include <textos/printk.h>

void sys_test (int rdi, int rsi, int rdx, int rcx, int r8, int r9)
{
    printk ("syscall test -> \n");
    printk (" args : rdi=0x%016llx rsi=0x%016llx\n"
            "        rdx=0x%016llx rcx=0x%016llx\n"
            "        r8 =0x%016llx r9 =0x%016llx\n",
            rdi, rsi, rdx, rcx, r8, r9);
    printk ("syscall test <-\n");
}

