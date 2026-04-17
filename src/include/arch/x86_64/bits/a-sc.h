typedef long long __sc_type;

#define __syscall(num, a1, a2, a3, a4, a5, a6)                                \
    ({                                                                        \
        register __sc_type __r0 asm("rax") = (__sc_type)(num);                \
        register __sc_type __r1 asm("rdi") = (__sc_type)(a1);                 \
        register __sc_type __r2 asm("rsi") = (__sc_type)(a2);                 \
        register __sc_type __r3 asm("rdx") = (__sc_type)(a3);                 \
        register __sc_type __r4 asm("r10") = (__sc_type)(a4);                 \
        register __sc_type __r5 asm("r8") = (__sc_type)(a5);                  \
        register __sc_type __r6 asm("r9") = (__sc_type)(a6);                  \
                                                                              \
        asm volatile("syscall"                                                \
                     : "+r"(__r0)                                             \
                     : "r"(__r1), "r"(__r2), "r"(__r3), "r"(__r4), "r"(__r5), \
                       "r"(__r6)                                              \
                     : "memory", "rcx", "r11");                               \
        __r0;                                                                 \
    })
